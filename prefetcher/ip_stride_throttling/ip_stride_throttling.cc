#include <algorithm>
#include <array>
#include <map>

#include "cache.h"

int PREFETCH_DEGREE = 4;
constexpr double threshold_1 = 0.3;
constexpr double threshold_2 = 0.7;
uint64_t counter = 0;
constexpr uint64_t COUNTER_THRESHOLD = 4096;
uint64_t pf_issued_till_now = 0;
uint64_t pf_useful_till_now = 0;
uint64_t pf_issued_running = 0;
uint64_t pf_useful_running = 0;

struct tracker_entry {
  uint64_t ip = 0;              // the IP we're tracking
  uint64_t last_cl_addr = 0;    // the last address accessed by this IP
  int64_t last_stride = 0;      // the stride between the last two addresses accessed by this IP
  uint64_t last_used_cycle = 0; // use LRU to evict old IP trackers
};

struct lookahead_entry {
  uint64_t address = 0;
  int64_t stride = 0;
  int degree = 0; // degree remaining
};

constexpr std::size_t TRACKER_SETS = 256;
constexpr std::size_t TRACKER_WAYS = 4;
std::map<CACHE*, lookahead_entry> lookahead;
std::map<CACHE*, std::array<tracker_entry, TRACKER_SETS * TRACKER_WAYS>> trackers;

void CACHE::prefetcher_initialize() { std::cout << NAME << " IP-based stride throttling prefetcher" << std::endl; }

void CACHE::prefetcher_cycle_operate()
{
  // If a lookahead is active
  if(pf_issued == 0 ){
    pf_issued_till_now = 0;
    pf_useful_till_now = 0;
    pf_issued_running = 0;
    pf_useful_running = 0;
  }
  if (auto [old_pf_address, stride, degree] = lookahead[this]; degree > 0) {
    auto pf_address = old_pf_address + (stride << LOG2_BLOCK_SIZE);
    // If the next step would exceed the degree or run off the page, stop
    if (virtual_prefetch || (pf_address >> LOG2_PAGE_SIZE) == (old_pf_address >> LOG2_PAGE_SIZE)) {
      // check the MSHR occupancy to decide if we're going to prefetch to this
      // level or not
      // bool success = prefetch_line(0, 0, pf_address, (get_occupancy(0, pf_address) < get_size(0, pf_address) / 2), 0);
      bool success = prefetch_line(pf_address, (get_occupancy(0, pf_address) < get_size(0, pf_address) / 2), 0);
      if (success)
        lookahead[this] = {pf_address, stride, degree - 1};
      // If we fail, try again next cycle
    } else {
      lookahead[this] = {};
    }
  }
}

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in)
{
  uint64_t cl_addr = addr >> LOG2_BLOCK_SIZE;
  int64_t stride = 0;

  // get boundaries of tracking set
  auto set_begin = std::next(std::begin(trackers[this]), ip % TRACKER_SETS);
  auto set_end = std::next(set_begin, TRACKER_WAYS);

  // find the current ip within the set
  auto found = std::find_if(set_begin, set_end, [ip](tracker_entry x) { return x.ip == ip; });

  // if we found a matching entry
  if (found != set_end) {
    // calculate the stride between the current address and the last address
    // no need to check for overflow since these values are downshifted
    stride = (int64_t)cl_addr - (int64_t)found->last_cl_addr;

    // Initialize prefetch state unless we somehow saw the same address twice in
    // a row or if this is the first time we've seen this stride
    if (stride != 0 && stride == found->last_stride)
      lookahead[this] = {cl_addr << LOG2_BLOCK_SIZE, stride, PREFETCH_DEGREE};
  } else {
    // replace by LRU
    found = std::min_element(set_begin, set_end, [](tracker_entry x, tracker_entry y) { return x.last_used_cycle < y.last_used_cycle; });
  }

  // update tracking set
  *found = {ip, cl_addr, stride, current_cycle};

  // update prefetch degree based on prefetch accuracy after every COUNTER_THRESHOLD accesses
  if (++counter == COUNTER_THRESHOLD) {
    counter = 0;
    pf_issued_running = 0.5 * pf_issued_running + (pf_issued - pf_issued_till_now);
    pf_useful_running = 0.5 * pf_useful_running + (pf_useful - pf_useful_till_now);
    double accuracy = pf_issued_running == 0 ? 0.5 : (double)pf_useful_running / (double)pf_issued_running;
    // std::cout << "Accuracy: " << accuracy << " issued_run: " << pf_issued_running << " useful_run: " << pf_useful_running <<  " pf_issued: " << pf_issued << " pf_useful: " << pf_useful << " issued_till_now: " << pf_issued_till_now << " useful_till_now: " << pf_useful_till_now << std::endl;
    pf_issued_till_now = pf_issued;
    pf_useful_till_now = pf_useful;
    if (accuracy < threshold_1)
      PREFETCH_DEGREE = 2;
    else if (accuracy < threshold_2)
      PREFETCH_DEGREE = 4;
    else
      PREFETCH_DEGREE = 6;
  }
  return metadata_in;
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_final_stats() {}