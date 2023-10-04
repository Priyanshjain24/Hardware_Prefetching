#include <algorithm>
#include <array>
#include <map>

#include "cache.h"

constexpr int PREFETCH_DEGREE = 5;
constexpr int PREFETCH_DISTANCE = 3;
constexpr std::size_t STREAM_SIZE = 64;
constexpr int THRESHOLD = 16;


//Tracker Entry States
#define INVALID     0
#define ALLOCATED   1
#define TRAINING    2
#define MONITORING  3

//Stream Directions
#define FORWARD     1
#define BACKWARD    -1

struct tracker_entry {
  uint64_t start_ptr = 0;
  uint64_t end_ptr = 0;
  int direction = BACKWARD;      
  uint32_t state = INVALID;     
  uint64_t original_addr = 0;   // the address that started this stream
  uint64_t last_used_cycle = 0; // use LRU to evict old IP trackers
};

struct lookahead_entry {
  uint64_t address = 0;
  int direction = BACKWARD;
  int degree = 0; // degree remaining
};

std::map<CACHE*, lookahead_entry> lookahead;
std::map<CACHE*, std::array<tracker_entry, STREAM_SIZE>> trackers;

void CACHE::prefetcher_initialize() { std::cout << NAME << " Stream Prefetcher" << std::endl; }

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in)
{
  uint64_t cl_addr = addr >> LOG2_BLOCK_SIZE;
  auto set_begin = std::begin(trackers[this]);
  auto set_end = std::end(trackers[this]);

  //  FINDING THE ENTRY IN MONITORING STATE
  auto monitor_entry = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) { return x.state == MONITORING && x.start_ptr <= cl_addr && x.end_ptr >= cl_addr; });
  if (monitor_entry != set_end) {
    // set lookahead
    if (monitor_entry->direction == FORWARD) {
      lookahead[this] = {monitor_entry->end_ptr << LOG2_BLOCK_SIZE, monitor_entry->direction, PREFETCH_DEGREE};
      monitor_entry->start_ptr = monitor_entry->start_ptr + PREFETCH_DEGREE;
      monitor_entry->end_ptr = monitor_entry->end_ptr + PREFETCH_DEGREE;
    } else {
      lookahead[this] = {monitor_entry->start_ptr << LOG2_BLOCK_SIZE, monitor_entry->direction, PREFETCH_DEGREE};
      monitor_entry->start_ptr = monitor_entry->start_ptr - PREFETCH_DEGREE;
      monitor_entry->end_ptr = monitor_entry->end_ptr - PREFETCH_DEGREE;
    }
    monitor_entry->last_used_cycle = current_cycle;
    return metadata_in;
  }


  // FINDING THE ENTRY IN TRAINING STATE
  auto training_entry = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) {
    return x.state == TRAINING && x.original_addr - THRESHOLD <= cl_addr && x.original_addr + THRESHOLD >= cl_addr;
  });
  if (training_entry != set_end) {
    // new direction
    int new_direction = (training_entry->original_addr < cl_addr) ? FORWARD : BACKWARD;
    if (new_direction == training_entry->direction) {
      training_entry->state = MONITORING;
      training_entry->start_ptr = training_entry->original_addr;
      // TODO: may change
      training_entry->end_ptr = training_entry->start_ptr + PREFETCH_DISTANCE;
    } else {
      training_entry->direction = new_direction;
    }
    training_entry->last_used_cycle = current_cycle;
    return metadata_in;
  }


  // FINDING THE ENTRY IN ALLOCATED STATE
  auto allocated_entry = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) {
    return x.state == ALLOCATED && x.original_addr - THRESHOLD <= cl_addr && x.original_addr + THRESHOLD >= cl_addr;
  });
  if (allocated_entry != set_end) {
    allocated_entry->direction = (allocated_entry->original_addr < cl_addr) ? FORWARD : BACKWARD;
    allocated_entry->state = TRAINING;
    allocated_entry->last_used_cycle = current_cycle;
    return metadata_in;
  }


  // FINDING THE ENTRY IN INVALID STATE
  auto invalid_entry = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) { return x.state == INVALID; });

  if (invalid_entry != set_end) 
  {
    invalid_entry->state = ALLOCATED;
    invalid_entry->original_addr = cl_addr;
    invalid_entry->last_used_cycle = current_cycle;
    return metadata_in;
  }

  // evict
  auto evict_block = std::min_element(set_begin, set_end, [](tracker_entry x, tracker_entry y) { return x.last_used_cycle < y.last_used_cycle; });
  evict_block->start_ptr = 0;
  evict_block->end_ptr = 0;
  evict_block->direction = BACKWARD;
  evict_block->state = ALLOCATED;
  evict_block->original_addr = cl_addr;
  evict_block->last_used_cycle = current_cycle;
  return metadata_in;
}

void CACHE::prefetcher_cycle_operate()
{
  if (auto [old_pf_address, direction, degree] = lookahead[this]; degree > 0) {
    auto pf_address = old_pf_address + (direction << LOG2_BLOCK_SIZE);
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