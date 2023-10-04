#include <algorithm>
#include <array>
#include <map>

#include "cache.h"

int PREFETCH_DEGREE = 5;
int PREFETCH_DISTANCE = 3;

#define THRESHOLD_1 = 0.2;
#define THRESHOLD_2 = 0.4;
#define THRESHOLD_3 = 0.6;
#define THRESHOLD_4 = 0.8;

uint64_t _counter = 0;
constexpr uint64_t _counter_THRESHOLD = 4096;

uint64_t Total_prefetches_issued = 0;
uint64_t Total_running_prefetches = 0;
uint64_t Current_issued_pf = 0;
uint64_t Current_useful_pf = 0;

constexpr std::size_t STREAM_SIZE = 64;
constexpr int THRESHOLD = 16;

//Tracker Entry States
#define INVALID     0
#define ALLOCATED   1
#define TRAINING    2
#define MONITORING  3 

//Direction States
#define FORWARD    1
#define DEFAULT    0
#define BACKWARD  -1

struct tracker_entry {
  uint64_t start_ptr = 0;
  uint64_t end_ptr = 0;
  int direction = DEFAULT;           
  uint32_t state = INVALID;      
  uint64_t original_addr = 0;   // the address that started this stream
  uint64_t last_used_cycle = 0; // use LRU to evict old IP trackers
};

struct lookahead_entry {
  uint64_t address = 0;
  int direction = DEFAULT;
  int degree = 0; // degree remaining
};

std::map<CACHE*, lookahead_entry> lookahead;
std::map<CACHE*, std::array<tracker_entry, STREAM_SIZE>> trackers;

void CACHE::prefetcher_initialize() { std::cout << NAME << " Stream throttling prefetcher" << std::endl; }

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in)
{
  // Update the prefetcher's degree and distance based on the prefetcher's accuracy
  if (++_counter == _counter_THRESHOLD) {
    _counter = 0;
    Current_issued_pf = 0.5 * Current_issued_pf + (pf_issued - Total_prefetches_issued);
    Current_useful_pf = 0.5 * Current_useful_pf + (pf_useful - Total_running_prefetches);
    Total_prefetches_issued = pf_issued;
    Total_running_prefetches = pf_useful;
    double accuracy = Current_issued_pf == 0 ? 0.5 : (double)Current_useful_pf / (double)Current_issued_pf;
    if (accuracy < THRESHOLD_1) 
    {
      PREFETCH_DEGREE = 1;
      PREFETCH_DISTANCE = 1;
    } 
    else if (accuracy < THRESHOLD_2) 
    {
      PREFETCH_DEGREE = 2;
      PREFETCH_DISTANCE = 2;
    } 
    else if (accuracy < THRESHOLD_3) 
    {
      PREFETCH_DEGREE = 2;
      PREFETCH_DISTANCE = 3;
    } 
    else if (accuracy < THRESHOLD_4) 
    {
      PREFETCH_DEGREE = 3;
      PREFETCH_DISTANCE = 3;
    } 
    else 
    {
      PREFETCH_DEGREE = 3;
      PREFETCH_DISTANCE = 4;
    }
  }

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
    // new direction
    int new_direction = (allocated_entry->original_addr < cl_addr) ? FORWARD : BACKWARD;
    allocated_entry->direction = new_direction;
    allocated_entry->state = TRAINING;
    allocated_entry->last_used_cycle = current_cycle;
    return metadata_in;
  }
  // FINDING THE ENTRY IN INVALID STATE
  auto invalid_entry = std::find_if(set_begin, set_end, [cl_addr](tracker_entry x) { return x.state == INVALID; });
  if (invalid_entry != set_end) {
    invalid_entry->state = ALLOCATED;
    invalid_entry->original_addr = cl_addr;
    invalid_entry->last_used_cycle = current_cycle;
    return metadata_in;
  }
  
  // LRU replaces the Least Recently Used Entry with Current Entry if their is NO entry already present in the set
  auto evict_block = std::min_element(set_begin, set_end, [](tracker_entry x, tracker_entry y) { return x.last_used_cycle < y.last_used_cycle; });
  evict_block->start_ptr = 0;
  evict_block->end_ptr = 0;
  evict_block->direction = DEFAULT;
  evict_block->state = ALLOCATED;
  evict_block->original_addr = cl_addr;
  evict_block->last_used_cycle = current_cycle;
  return metadata_in;
}

void CACHE::prefetcher_cycle_operate()
{
  if(pf_issued == 0 ){
    Total_prefetches_issued = 0;
    Total_running_prefetches = 0;
    Current_issued_pf = 0;
    Current_useful_pf = 0;
  }
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