#ifndef __UTILS_SPMU_H__
#define __UTILS_SPMU_H__
#include "common.h"

extern uint64_t branch_inst;
extern uint8_t br_type;
extern uint8_t pre_taken;
extern uint32_t pre_target;
extern uint8_t true_taken;
extern uint32_t true_target;
extern uint64_t typeb_br;
extern uint64_t ret_inst;
extern uint64_t ret_error_inst;
extern uint64_t id_branch_error;
extern uint64_t exe_branch_error;
extern uint64_t mem_branch_error;
extern uint64_t dcache_hit, dcache_miss;
extern uint64_t icache_hit, icache_miss;

#endif