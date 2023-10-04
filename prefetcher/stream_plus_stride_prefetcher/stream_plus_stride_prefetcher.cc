#include <algorithm>
#include <array>
#include <map>

#include "cache.h"

constexpr int PREFETCH_DEGREE = 3;
constexpr int PREFETCH_DISTANCE = 3;
constexpr std::size_t STREAM_SIZE = 64;
constexpr int THRESHOLD = 16;

struct tracker_entry {
  uint64_t start_ptr = 0;
  uint64_t end_ptr = 0;
  int stride = 0;            // -ve = backwards, +ve = forwards
  uint32_t state = 0;           // 0 = invalid, 1 = allocated, 2 = training, 3 = monitoring
  uint64_t original_addr = 0;   // the address that started this stream
  uint64_t last_accesed_addr = 0;
  uint64_t last_used_cycle = 0; // use LRU to evict old IP trackers
};

struct lookahead_entry {
  uint64_t address = 0;
  int stride = 0;
  int degree = 0; // degree remaining
};

std::map<CACHE*, lookahead_entry> lookahead;
std::map<CACHE*, std::array<tracker_entry, STREAM_SIZE>> trackers;

void CACHE::prefetcher_initialize() { std::cout << NAME << " Stream plus stride based prefetcher" << std::endl; }

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in)
{
  uint64_t cl_addr = addr >> LOG2_BLOCK_SIZE;
  auto set_begin = std::begin(trackers[this]);
  auto set_end = std::end(trackers[this]);
  // monitor found
  auto found_monitor = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) { return x.state == 3 && x.start_ptr <= cl_addr && x.end_ptr >= cl_addr; });
  if (found_monitor != set_end) {
    // set lookahead
    if (found_monitor->stride > 0) {
      lookahead[this] = {found_monitor->end_ptr << LOG2_BLOCK_SIZE, found_monitor->stride, PREFETCH_DEGREE};
      found_monitor->start_ptr = found_monitor->start_ptr + PREFETCH_DEGREE;
      found_monitor->end_ptr = found_monitor->end_ptr + PREFETCH_DEGREE;
    } else {
      lookahead[this] = {found_monitor->start_ptr << LOG2_BLOCK_SIZE, found_monitor->stride, PREFETCH_DEGREE};
      found_monitor->start_ptr = found_monitor->start_ptr - PREFETCH_DEGREE;
      found_monitor->end_ptr = found_monitor->end_ptr - PREFETCH_DEGREE;
    }
    found_monitor->last_used_cycle = current_cycle;
    return metadata_in;
  }
  if((int)cache_hit == 1) return metadata_in;
  // training found
  auto found_training = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) {
    return x.state == 2 && x.original_addr - THRESHOLD <= cl_addr && x.original_addr + THRESHOLD >= cl_addr;
  });
  if (found_training != set_end) {
    // new stride
    int new_stride = cl_addr - found_training->last_accesed_addr;
    if (new_stride == found_training->stride) {
      found_training->state = 3;
      found_training->start_ptr = found_training->original_addr;
      // TODO: may change
      found_training->end_ptr = found_training->start_ptr + PREFETCH_DISTANCE;
    } else {
      found_training->stride = new_stride;
      found_training->last_accesed_addr = cl_addr;
    }
    found_training->last_used_cycle = current_cycle;
    return metadata_in;
  }
  // allocated found
  auto found_allocated = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) {
    return x.state == 1 && x.original_addr - THRESHOLD <= cl_addr && x.original_addr + THRESHOLD >= cl_addr;
  });
  if (found_allocated != set_end) {
    // new stride
    int new_stride = cl_addr - found_allocated->last_accesed_addr;
    found_allocated->stride = new_stride;
    found_allocated->state = 2;
    found_allocated->last_accesed_addr = cl_addr;
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
  evict->stride = 0;
  evict->state = 1;
  evict->original_addr = cl_addr;
  evict->last_used_cycle = current_cycle;
  return metadata_in;
}

void CACHE::prefetcher_cycle_operate()
{
  if (auto [old_pf_address, stride, degree] = lookahead[this]; degree > 0) {
    auto pf_address = old_pf_address + (stride << LOG2_BLOCK_SIZE);
    // If the next step would exceed the degree or run off the page, stop
    if (virtual_prefetch || (pf_address >> LOG2_PAGE_SIZE) == (old_pf_address >> LOG2_PAGE_SIZE)) {
      bool success = prefetch_line(pf_address, (get_occupancy(0, pf_address) < get_size(0, pf_address) / 2), 0);
      if (success)
        lookahead[this] = {pf_address, stride, degree - 1};
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