package RVNoob.Cache

import RVNoob.Core.{Judge_Load_op, MemCtrlIO, RVNoobConfig, ext_function}
import chisel3._
import chisel3.util.MuxCase

// Data Mem, have discarded
class DATAM extends Module with RVNoobConfig {
  val io = IO(new Bundle {
    val valid     = Input(Bool())
    val data_addr = Input(UInt(xlen.W))
    val wdata     = Input(UInt(xlen.W))
    val rdata     = Output(UInt(xlen.W))
    val mem_ctrl  = Input(new MemCtrlIO)
  })

  val daddr = io.data_addr & (~0x7.U(64.W)).asUInt
  val shift = Wire(UInt(3.W))
  val rdata = Wire(UInt(xlen.W))
  val wdata = Wire(UInt(xlen.W))
  val wmask = Wire(UInt(8.W))

  shift    := io.data_addr
  io.rdata := (rdata >> (shift * 8.U))
  wdata    := (io.wdata << (shift * 8.U))
  wmask := MuxCase(
    "b11111111".U,
    Array(
      (io.mem_ctrl.zero_ex_op === 3.U) -> "b11111111".U, // write double word
      (io.mem_ctrl.zero_ex_op === 2.U) -> ("b1111".U << shift), // write word
      (io.mem_ctrl.zero_ex_op === 1.U) -> ("b11".U << shift), // write half word
      (io.mem_ctrl.zero_ex_op === 0.U) -> ("b1".U << shift) // write byte
    )
  )

  val dpi_pmem = Module(new DpiPmem)
  dpi_pmem.io.clk    <> clock
  dpi_pmem.io.raddr  <> daddr
  dpi_pmem.io.waddr  <> daddr
  dpi_pmem.io.wmask  <> wmask
  dpi_pmem.io.rdata  <> rdata
  dpi_pmem.io.wdata  <> wdata
  dpi_pmem.io.r_pmem <> io.mem_ctrl.r_pmem
  dpi_pmem.io.w_pmem <> (io.mem_ctrl.w_pmem && io.valid)

  // assert
  when(io.mem_ctrl.r_pmem || io.mem_ctrl.w_pmem) {
    when(io.mem_ctrl.zero_ex_op === 3.U) {
      assert(shift === 0.U)
    }.elsewhen(io.mem_ctrl.zero_ex_op === 2.U) {
      assert(shift <= 4.U)
    }.elsewhen(io.mem_ctrl.zero_ex_op === 1.U) {
      assert(shift <= 6.U)
    }.otherwise {
      assert(shift < 8.U)
    }
  }
}

// DPI implemented, Main Mem access module
class DpiPmem extends BlackBox with RVNoobConfig {
  val io = IO(new Bundle {
    val clk    = Input(Clock())
    val pc     = Input(UInt(64.W))
    val raddr  = Input(UInt(xlen.W))
    val rdata  = Output(UInt(xlen.W))
    val r_pmem = Input(Bool())

    val waddr  = Input(UInt(xlen.W))
    val wmask  = Input(UInt((xlen / 8).W))
    val wdata  = Input(UInt(xlen.W))
    val w_pmem = Input(Bool())
  })
}

// Combination logic, after wb reg to deal with dcache.rdata
class JudgeLoad extends Module with RVNoobConfig with ext_function with Judge_Load_op {
  val io = IO(new Bundle {
    val mem_data      = Input(UInt(xlen.W))
    val judge_load_op = Input(UInt(jdgl_op_w.W))
    val load_data     = Output(UInt(xlen.W))
  })

  io.load_data := MuxCase(
    io.mem_data,
    Array(
      (io.judge_load_op === jlop_sextw) -> sext_64(io.mem_data(31, 0)),
      (io.judge_load_op === jlop_sexthw) -> sext_64(io.mem_data(15, 0)),
      (io.judge_load_op === jlop_sextb) -> sext_64(io.mem_data(7, 0)),
      (io.judge_load_op === jlop_uextw) -> uext_64(io.mem_data(31, 0)),
      (io.judge_load_op === jlop_uexthw) -> uext_64(io.mem_data(15, 0)),
      (io.judge_load_op === jlop_uextb) -> uext_64(io.mem_data(7, 0))
    )
  )
  override def desiredName = if (tapeout) ysyxid + "_" + getClassName else getClassName

}