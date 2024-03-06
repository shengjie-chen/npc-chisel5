#ifndef __MEMORY_MEMORY_H__
#define __MEMORY_MEMORY_H__

#include "common.h"

bool in_pmem(paddr_t addr);
uint8_t *guest_to_host(paddr_t paddr);
long load_img();

extern char *img_file;
extern uint8_t pmem[CONFIG_MSIZE] PG_ALIGN;

const uint32_t img[] = {
    // 测试用例中包括跳转指令，但不存在指令相关性冲突
    0x00000413, // li	s0,0              0x80000000
    0x00009117, // auipc	sp,0x9        0x80000004
    0x01858593, // addi	a1,a1,24        0x80000008
    0x004c8c93, // addi	s9,s9,4         0x8000000c
    0x02040a63, // beqz	s0,0x34         0x80000010 -> 0x80000044
    // 0x00840413, // addi	s0,s0,8
    0x004a0a13, // addi	s4,s4,4         0x80000014
    0xffc10113, // addi	sp,sp,-4        0x80000018
    0x01858593, // addi	a1,a1,24        0x8000001c
    0x004c8c93, // addi	s9,s9,4         0x80000020
    0x00840413, // addi	s0,s0,8         0x80000024
    0x004a0a13, // addi	s4,s4,4         0x80000028
    0xff010113, // addi	sp,sp,-16       0x8000002c
    0x01858593, // addi	a1,a1,24        0x80000030
    0x004c8c93, // addi	s9,s9,4         0x80000034
    0x00840413, // addi	s0,s0,8         0x80000038
    0x004a0a13, // addi	s4,s4,4         0x8000003c
    0x00b13423, // sd	a1,8(sp)          0x80000040
    0x00006597, // auipc	a1,0x6        0x80000044
    0x00000517, // auipc	a0,0x0        0x80000048
    0x0015869b, // addiw	a3,a1,1       0x8000004c
    0xff010113, // addi	sp,sp,-16       0x80000050
    0x01c50513, // addi	a0,a0,28
    0x004c8c93, // addi	s9,s9,4
    0x00050513, // mv	a0,a0
    0x00813983, // ld	s3,8(sp)
    0x00100073  // ebreak
    //  0x0102b503,  // ld  a0,16(t0)
    //  0x00100073,  // ebreak (used as NPC_trap)
    //  0xdeadbeef,  // some data
};

// const uint32_t img[] = {
//     0xff010113, // addi	sp,sp,-16
//     0xfa010113, // addi	sp,sp,-96
//     0x004c8c93, // addi	s9,s9,4
//     0x00840413, // addi	s0,s0,8
//     0x004a0a13, // addi	s4,s4,4
//     0xff010113, // addi	sp,sp,-16
//     0xfa010113, // addi	sp,sp,-96
//     0x004c8c93, // addi	s9,s9,4
//     0x00840413, // addi	s0,s0,8
//     0x004a0a13, // addi	s4,s4,4
//     0x00100073  // ebreak
//     //  0x0102b503,  // ld  a0,16(t0)
//     //  0x00100073,  // ebreak (used as NPC_trap)
//     //  0xdeadbeef,  // some data
// };
//  const uint32_t img[] = { // 测试用例中不包括跳转指令，且不存在指令相关性冲突
//      0x00000413, // li	s0,0
//      0x00009117, // auipc	sp,0x9
//      0x01858593, // addi	a1,a1,24
//      0x004c8c93, // addi	s9,s9,4
//      0x00840413, // addi	s0,s0,8
//      0x004a0a13, // addi	s4,s4,4
//      0xffc10113, // addi	sp,sp,-4
//      0x01858593, // addi	a1,a1,24
//      0x004c8c93, // addi	s9,s9,4
//      0x00840413, // addi	s0,s0,8
//      0x004a0a13, // addi	s4,s4,4
//      0xff010113, // addi	sp,sp,-16
//      0x01858593, // addi	a1,a1,24
//      0x004c8c93, // addi	s9,s9,4
//      0x00840413, // addi	s0,s0,8
//      0x004a0a13, // addi	s4,s4,4
//      0x00b13423, // sd	a1,8(sp)
//      0x00006597, // auipc	a1,0x6
//      0x00000517, // auipc	a0,0x0
//      0x0015869b, // addiw	a3,a1,1
//      0xff010113, // addi	sp,sp,-16
//      0x01c50513, // addi	a0,a0,28
//      0x004c8c93, // addi	s9,s9,4
//      0x00050513, // mv	a0,a0
//      0x00813983, // ld	s3,8(sp)
//      0x00100073  // ebreak
//      //  0x0102b503,  // ld  a0,16(t0)
//      //  0x00100073,  // ebreak (used as NPC_trap)
//      //  0xdeadbeef,  // some data
//  };

#endif