#include <algorithm>
#include <array>
#include <map>

#include "cache.h"

int PREFETCH_DEGREE = 5;
int PREFETCH_DISTANCE = 3;
constexpr double threshold_1 = 0.2;
constexpr double threshold_2 = 0.4;
constexpr double threshold_3 = 0.6;
constexpr double threshold_4 = 0.8;
uint64_t counter = 0;
constexpr uint64_t COUNTER_THRESHOLD = 4096;
uint64_t pf_issued_till_now = 0;
uint64_t pf_useful_till_now = 0;
uint64_t pf_issued_running = 0;
uint64_t pf_useful_running = 0;
constexpr std::size_t STREAM_SIZE = 64;
constexpr int THRESHOLD = 16;

struct tracker_entry {
  uint64_t start_ptr = 0;
  uint64_t end_ptr = 0;
  int direction = 0;            // false = backwards, true = forwards
  uint32_t state = 0;           // 0 = invalid, 1 = allocated, 2 = training, 3 = monitoring
  uint64_t original_addr = 0;   // the address that started this stream
  uint64_t last_used_cycle = 0; // use LRU to evict old IP trackers
};

struct lookahead_entry {
  uint64_t address = 0;
  int direction = 0;
  int degree = 0; // degree remaining
};

std::map<CACHE*, lookahead_entry> lookahead;
std::map<CACHE*, std::array<tracker_entry, STREAM_SIZE>> trackers;

void CACHE::prefetcher_initialize() { std::cout << NAME << " Stream throttling prefetcher" << std::endl; }

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in)
{
  // Update the prefetcher's degree and distance based on the prefetcher's accuracy
  if (++counter == COUNTER_THRESHOLD) {
    counter = 0;
    pf_issued_running = 0.5 * pf_issued_running + (pf_issued - pf_issued_till_now);
    pf_useful_running = 0.5 * pf_useful_running + (pf_useful - pf_useful_till_now);
    pf_issued_till_now = pf_issued;
    pf_useful_till_now = pf_useful;
    double accuracy = pf_issued_running == 0 ? 0.5 : (double)pf_useful_running / (double)pf_issued_running;
    if (accuracy < threshold_1) {
      PREFETCH_DEGREE = 1;
      PREFETCH_DISTANCE = 1;
    } else if (accuracy < threshold_2) {
      PREFETCH_DEGREE = 2;
      PREFETCH_DISTANCE = 2;
    } else if (accuracy < threshold_3) {
      PREFETCH_DEGREE = 2;
      PREFETCH_DISTANCE = 3;
    } else if (accuracy < threshold_4) {
      PREFETCH_DEGREE = 3;
      PREFETCH_DISTANCE = 3;
    } else {
      PREFETCH_DEGREE = 3;
      PREFETCH_DISTANCE = 4;
    }
  }

  uint64_t cl_addr = addr >> LOG2_BLOCK_SIZE;
  auto set_begin = std::begin(trackers[this]);
  auto set_end = std::end(trackers[this]);
  // monitor found
  auto found_monitor = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) { return x.state == 3 && x.start_ptr <= cl_addr && x.end_ptr >= cl_addr; });
  if (found_monitor != set_end) {
    // set lookahead
    if (found_monitor->direction == 1) {
      lookahead[this] = {found_monitor->end_ptr << LOG2_BLOCK_SIZE, found_monitor->direction, PREFETCH_DEGREE};
      found_monitor->start_ptr = found_monitor->start_ptr + PREFETCH_DEGREE;
      found_monitor->end_ptr = found_monitor->end_ptr + PREFETCH_DEGREE;
    } else {
      lookahead[this] = {found_monitor->start_ptr << LOG2_BLOCK_SIZE, found_monitor->direction, PREFETCH_DEGREE};
      found_monitor->start_ptr = found_monitor->start_ptr - PREFETCH_DEGREE;
      found_monitor->end_ptr = found_monitor->end_ptr - PREFETCH_DEGREE;
    }
    found_monitor->last_used_cycle = current_cycle;
    return metadata_in;
  }
  // if(cache_hit) return metadata_in;
  // training found
  auto found_training = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) {
    return x.state == 2 && x.original_addr - THRESHOLD <= cl_addr && x.original_addr + THRESHOLD >= cl_addr;
  });
  if (found_training != set_end) {
    // new direction
    int new_direction = (found_training->original_addr < cl_addr) ? 1 : -1;
    if (new_direction == found_training->direction) {
      found_training->state = 3;
      found_training->start_ptr = found_training->original_addr;
      // TODO: may change
      found_training->end_ptr = found_training->start_ptr + PREFETCH_DISTANCE;
    } else {
      found_training->direction = new_direction;
    }
    found_training->last_used_cycle = current_cycle;
    return metadata_in;
  }
  // allocated found
  auto found_allocated = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) {
    return x.state == 1 && x.original_addr - THRESHOLD <= cl_addr && x.original_addr + THRESHOLD >= cl_addr;
  });
  if (found_allocated != set_end) {
    // new direction
    int new_direction = (found_allocated->original_addr < cl_addr) ? 1 : -1;
    found_allocated->direction = new_direction;
    found_allocated->state = 2;
    found_allocated->last_used_cycle = current_cycle;
    return metadata_in;
  }
  // invalid found
  auto found_invalid = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) { return x.state == 0; });
  if (found_invalid != set_end) {
    found_invalid->state = 1;
    found_invalid->original_addr = cl_addr;
    found_invalid->last_used_cycle = current_cycle;
    return metadata_in;
  }
  // evict
  auto evict = std::min_element(set_begin, set_end, [](tracker_entry x, tracker_entry y) { return x.last_used_cycle < y.last_used_cycle; });
  evict->start_ptr = 0;
  evict->end_ptr = 0;
  evict->direction = 0;
  evict->state = 1;
  evict->original_addr = cl_addr;
  evict->last_used_cycle = current_cycle;
  return metadata_in;
}

void CACHE::prefetcher_cycle_operate()
{
  if(pf_issued == 0 ){
    pf_issued_till_now = 0;
    pf_useful_till_now = 0;
    pf_issued_running = 0;
    pf_useful_running = 0;
  }
  if (auto [old_pf_address, direction, degree] = lookahead[this]; degree > 0) {
    auto pf_address = old_pf_address + (direction << LOG2_BLOCK_SIZE);
    // If the next step would exceed the degree or run off the page, stop
    if (virtual_prefetch || (pf_address >> LOG2_PAGE_SIZE) == (old_pf_address >> LOG2_PAGE_SIZE)) {
      bool success = prefetch_line(pf_address, (get_occupancy(0, pf_address) < get_size(0, pf_address) / 2), 0);
      if (success)
        lookahead[this] = {pf_address, direction, degree - 1};
    } else {
      lookahead[this] = {};
    }
  }
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_final_stats() {}