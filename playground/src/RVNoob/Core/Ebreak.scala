package RVNoob.Core

import chisel3._

class DpiEbreak extends BlackBox {
  val io = IO(new Bundle {
    val clk    = Input(Clock())
    val inst   = Input(UInt(32.W))
    val a0     = Input(UInt(64.W))
  })

}

object DpiEbreak {
  def apply(
    clk:    Clock,
    inst:   UInt,
    a0:     UInt,
  ): DpiEbreak = {
    val u_ebreak = Module(new DpiEbreak)
    u_ebreak.io.clk    <> clk
    u_ebreak.io.inst   <> inst
    u_ebreak.io.a0     <> a0
    u_ebreak
  }
}
