package RVNoob.Core

import RVNoob.Axi.AxiSlaveMem
import chisel3._

class RVNoobSim extends Module with ext_function with RVNoobConfig {
  val io = IO(new Bundle {
    val pc        = Output(UInt(addr_w.W))
    val ebreak    = Output(Bool())
    val diff_en   = Output(Bool())
    val diff_pc   = Output(UInt(addr_w.W))
    val diff_inst = Output(UInt(inst_w.W))
    val inst_cnt  = Output(UInt(xlen.W))
  })

  val rvnoob = RVNoobTile()

  // >>>>>>>>>>>>>> SAXI <<<<<<<<<<<<<<
  val axi_pmem = Module(new AxiSlaveMem)

  // >>>>>>>>>>>>>> SAXI <<<<<<<<<<<<<<
  axi_pmem.io.S_AXI_ACLK    <> clock
  axi_pmem.io.S_AXI_ARESETN <> !reset.asBool
  axi_pmem.io.S_AXI_AWVALID <> rvnoob.io.master.awvalid
  axi_pmem.io.S_AXI_AWREADY <> rvnoob.io.master.awready
  axi_pmem.io.S_AXI_AWID    <> rvnoob.io.master.awid
  axi_pmem.io.S_AXI_AWADDR  <> rvnoob.io.master.awaddr
  axi_pmem.io.S_AXI_AWLEN   <> rvnoob.io.master.awlen
  axi_pmem.io.S_AXI_AWSIZE  <> rvnoob.io.master.awsize
  axi_pmem.io.S_AXI_AWBURST <> rvnoob.io.master.awburst
  axi_pmem.io.S_AXI_WVALID  <> rvnoob.io.master.wvalid
  axi_pmem.io.S_AXI_WREADY  <> rvnoob.io.master.wready
  axi_pmem.io.S_AXI_WDATA   <> rvnoob.io.master.wdata
  axi_pmem.io.S_AXI_WSTRB   <> rvnoob.io.master.wstrb
  axi_pmem.io.S_AXI_WLAST   <> rvnoob.io.master.wlast
  axi_pmem.io.S_AXI_BVALID  <> rvnoob.io.master.bvalid
  axi_pmem.io.S_AXI_BREADY  <> rvnoob.io.master.bready
  axi_pmem.io.S_AXI_BID     <> rvnoob.io.master.bid
  axi_pmem.io.S_AXI_BRESP   <> rvnoob.io.master.bresp
  axi_pmem.io.S_AXI_ARVALID <> rvnoob.io.master.arvalid
  axi_pmem.io.S_AXI_ARREADY <> rvnoob.io.master.arready
  axi_pmem.io.S_AXI_ARID    <> rvnoob.io.master.arid
  axi_pmem.io.S_AXI_ARADDR  <> rvnoob.io.master.araddr
  axi_pmem.io.S_AXI_ARLEN   <> rvnoob.io.master.arlen
  axi_pmem.io.S_AXI_ARSIZE  <> rvnoob.io.master.arsize
  axi_pmem.io.S_AXI_ARBURST <> rvnoob.io.master.arburst
  axi_pmem.io.S_AXI_RVALID  <> rvnoob.io.master.rvalid
  axi_pmem.io.S_AXI_RREADY  <> rvnoob.io.master.rready
  axi_pmem.io.S_AXI_RID     <> rvnoob.io.master.rid
  axi_pmem.io.S_AXI_RDATA   <> rvnoob.io.master.rdata
  axi_pmem.io.S_AXI_RRESP   <> rvnoob.io.master.rresp
  axi_pmem.io.S_AXI_RLAST   <> rvnoob.io.master.rlast

  axi_pmem.io.PC <> rvnoob.io.axi_pc

  io.pc        <> rvnoob.io.pc
  io.ebreak    <> rvnoob.io.ebreak
  io.diff_en   <> rvnoob.io.diff_en
  io.diff_pc   <> rvnoob.io.diff_pc
  io.diff_inst <> rvnoob.io.diff_inst
  io.inst_cnt  <> rvnoob.io.inst_cnt

}
