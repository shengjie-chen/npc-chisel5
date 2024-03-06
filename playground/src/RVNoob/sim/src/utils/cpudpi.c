/*
 * @Author: Shengjie Chen chenshengjie1999@126.com
 * @Date: 2022-12-07 22:51:47
 * @LastEditors: Shengjie Chen chenshengjie1999@126.com
 * @LastEditTime: 2022-12-10 10:48:32
 * @FilePath: /npc/playground/src/RVnpc/RVNoob/useddpi.c
 * @Description: 用到的dpi变量和函数集合
 */

#include "common.h"

/// @brief 获取cpu的指令，当前得到idu的指令
uint32_t cpu_inst;
extern "C" void inst_change(const svLogicVecVal *r) {
    cpu_inst = *(uint32_t *)(r);
    // printf("inst : %x\n", cpu_inst);
}

/// @brief 获取pc值，即当周期pc寄存器的值; 获取下一个pc值，即下一周期pc寄存器的值; 获取当前执行了多少指令
uint32_t cpu_pc, cpu_npc, cpu_inst_cnt;
extern "C" void pc_change(const svLogicVecVal *a, const svLogicVecVal *b, const svLogicVecVal *c) {
    cpu_pc = *(vaddr_t *)(a);
    cpu_npc = *(vaddr_t *)(b);
    cpu_inst_cnt = *(uint64_t *)(c);
}

/// @brief 获取pc值，即当周期pc寄存器的值; 获取下一个pc值，即下一周期pc寄存器的值; 获取当前执行了多少指令
uint8_t diff_en;
uint32_t diff_pc, diff_inst;
extern "C" void difftest_change(svLogic a, const svLogicVecVal *b, const svLogicVecVal *c) {
    diff_en = a;
    diff_pc = *(vaddr_t *)(b);
    diff_inst = *(uint32_t *)(c);
}

extern "C" void npc_ebreak() {
    npc_state.state = NPC_END;
    printf("!!!!!! npc ebreak !!!!!!\n");
}

// uint32_t *cpu_inst = NULL;
// extern "C" void set_inst_ptr(const svLogicVecVal *r)
//{
//   cpu_inst = (uint32_t *)(r);
// }

/// @brief 获取mem_reg.out.pc
uint32_t mem_pc;
extern "C" void mem_pc_change(const svLogicVecVal *r) { mem_pc = *(vaddr_t *)(r); }

uint8_t wb_valid;
vaddr_t wb_pc;
uint32_t wb_inst;
extern "C" void wb_change(svLogic v, const svLogicVecVal *p, const svLogicVecVal *i) {
    wb_valid = v;
    wb_pc = *(vaddr_t *)(p);
    wb_inst = *(uint32_t *)(i);
}


