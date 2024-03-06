package RVNoob.Pipeline

import RVNoob.Branch._
import RVNoob.Core._
import chisel3._
import chisel3.util._

trait EXregSignal extends RVNoobConfig {
  val pc = UInt(addr_w.W)
//  val inst = UInt(if (tapeout) 0.W else inst_w.W)
  val inst = UInt(inst_w.W)

  val snpc     = UInt(addr_w.W)
  val src1     = UInt(xlen.W)
  val src2     = UInt(xlen.W)
  val imm      = UInt(xlen.W)
  val csr_dnpc = UInt(addr_w.W)

  val exe_ctrl    = new EXECtrlIO
  val mem_ctrl    = new MemCtrlIO
  val wb_rf_ctrl  = new WbRfCtrlIO
  val wb_csr_ctrl = new WbCsrCtrlIO
  val dnpc_ctrl   = new DnpcCtrlIO

  val br_pre  = new branch_pre
  val br_info = new branch_info
}

class EXregInIO extends PipelineInIO with EXregSignal {}

class EXregOutIO extends PipelineOutIO with EXregSignal {}

class EXreg extends Module with RVNoobConfig {
  val in  = IO(Input(new EXregInIO))
  val out = IO(Output(new EXregOutIO))
  if (!tapeout) {
    dontTouch(in)
    dontTouch(out)
  }

  out.pc   := RegEnable(in.pc, 0.U, in.reg_en)
  out.inst := RegEnable(in.inst, 0.U, in.reg_en)

  out.snpc     := RegEnable(in.snpc, 0.U, in.reg_en)
  out.src1     := RegEnable(in.src1, 0.U, in.reg_en)
  out.src2     := RegEnable(in.src2, 0.U, in.reg_en)
  out.imm      := RegEnable(in.imm, 0.U, in.reg_en)
  out.csr_dnpc := RegEnable(in.csr_dnpc, 0.U, in.reg_en)

  out.exe_ctrl    := RegEnable(in.exe_ctrl, 0.U.asTypeOf(new EXECtrlIO), in.reg_en)
  out.mem_ctrl    := RegEnable(in.mem_ctrl, 0.U.asTypeOf(new MemCtrlIO), in.reg_en)
  out.wb_rf_ctrl  := RegEnable(in.wb_rf_ctrl, 0.U.asTypeOf(new WbRfCtrlIO), in.reg_en)
  out.wb_csr_ctrl := RegEnable(in.wb_csr_ctrl, 0.U.asTypeOf(new WbCsrCtrlIO), in.reg_en)
  out.dnpc_ctrl   := RegEnable(in.dnpc_ctrl, 0.U.asTypeOf(new DnpcCtrlIO), in.reg_en)

  out.br_pre  := RegEnable(in.br_pre, 0.U.asTypeOf(new branch_pre), in.reg_en)
  out.br_info := RegEnable(in.br_info, 0.U.asTypeOf(new branch_info), in.reg_en)

  out.valid      := RegNext(in.reg_en && in.valid, 0.B)
  out.inst_valid := (out.inst =/= 0.U)

  override def desiredName = if (tapeout) ysyxid + "_" + getClassName else getClassName
}

//object EXreg {
//  def apply(
//    pc:          UInt,
//    inst:        UInt,
//    snpc:        UInt,
//    src1:        UInt,
//    src2:        UInt,
//    imm:         UInt,
//    csr_dnpc:    UInt,
//    exe_ctrl:    EXECtrlIO,
//    mem_ctrl:    MemCtrlIO,
//    wb_rf_ctrl:  WbRfCtrlIO,
//    wb_csr_ctrl: WbCsrCtrlIO,
//    dnpc_ctrl:   DnpcCtrlIO,
//    reg_en:      Bool,
//    valid:       Bool,
//    br_pre:      branch_pre
//  ): EXreg = {
//    val ex_reg = Module(new EXreg)
//    ex_reg.in.pc          <> pc
//    ex_reg.in.inst        <> inst
//    ex_reg.in.snpc        <> snpc
//    ex_reg.in.src1        <> src1
//    ex_reg.in.src2        <> src2
//    ex_reg.in.imm         <> imm
//    ex_reg.in.csr_dnpc    <> csr_dnpc
//    ex_reg.in.exe_ctrl    <> exe_ctrl
//    ex_reg.in.mem_ctrl    <> mem_ctrl
//    ex_reg.in.wb_rf_ctrl  <> wb_rf_ctrl
//    ex_reg.in.wb_csr_ctrl <> wb_csr_ctrl
//    ex_reg.in.dnpc_ctrl   <> dnpc_ctrl
//    ex_reg.in.br_pre      <> br_pre
//
//    ex_reg.in.reg_en <> reg_en
//    ex_reg.in.valid  <> valid
//
//    ex_reg
//  }
//}