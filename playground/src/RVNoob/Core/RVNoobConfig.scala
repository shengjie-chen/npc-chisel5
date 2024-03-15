package RVNoob.Core

import chisel3._
import chisel3.util._

import scala.math.pow

trait RVNoobModeConfig {
  // type : 0 - sim; 1 - tapeout; 2 - fpga
  val fpga:            Boolean = false
  val tapeout:         Boolean = true
  val spmu_en:         Boolean = false
  val soc_sim:         Boolean = false
  val simplify_design: Boolean = !tapeout && !soc_sim
  require(!(tapeout && soc_sim), "tapout and soc_sim can't be true at the same time")
  if (tapeout) {
    require(!spmu_en, "spmu_en must be false when tapeout is true")
  }
}

trait RVNoobConfig extends util_function with BranchConfig with RVNoobModeConfig {
  val alu_op_w   = 5 //alu_op_width
  val jdg_op_w   = 4 //judge_op_width
  val jdgl_op_w  = 3 //judge_load_op_width
  val xlen       = 64
  val inst_w     = 32
  val gpr_addr_w = 5
  val addr_w     = 32

//  val ICacheSize = 1
//  val DCacheSize = 0.5
  val ICacheSize = 4
  val DCacheSize = 4

  val ysyxid = "ysyx_22040495"
  def getClassName: String = this.getClass.toString.split("\\.").last
}

trait RVNoobMemMap extends RVNoobModeConfig {
  val mem_map =
    if (tapeout)
      Map(
        "reserve1" -> (0x00000000L.U, 0x01ffffffL.U),
        "clint"    -> (0x02000000L.U, 0x0200ffffL.U),
        "reserve2" -> (0x02010000L.U, 0x0fffffffL.U),
        "uart"     -> (0x10000000L.U, 0x10000fffL.U),
        "spi"      -> (0x10001000L.U, 0x10001fffL.U),
        "vga"      -> (0x10002000L.U, 0x10002fffL.U),
        "ps2"      -> (0x10003000L.U, 0x10003fffL.U),
        "ethernet" -> (0x10004000L.U, 0x10004fffL.U),
        "reserve3" -> (0x10005000L.U, 0x2fffffffL.U),
        "flash"    -> (0x30000000L.U, 0x3fffffffL.U),
        "chiplink" -> (0x40000000L.U, 0x7fffffffL.U),
        "mem"      -> (0x80000000L.U, 0xfbffffffL.U),
        "sdram"    -> (0xfc000000L.U, 0xffffffffL.U)
      )
    else if (soc_sim)
      Map(
        "sram" -> (0x0f000000L.U, 0x0f001fffL.U),
        "mrom" -> (0x02000000L.U, 0x02000fffL.U)
      )
    else
      Map(
        "clint"  -> (0x02000000L.U, 0x0200ffffL.U),
        "pmem"   -> (0x80000000L.U, 0x88000000L.U),
        "device" -> (0xa0000000L.U, 0xa1ffffffL.U)
      )

  def check_in_range(addr: UInt, device: String): Bool = {
    addr >= mem_map(device)._1 && addr <= mem_map(device)._2
  }

  def check_in_range(addr: UInt, size: UInt, device: String): Bool = {
    check_in_range(addr, device) && check_in_range(addr + size - 1.U, device)
  }

  def check_in_range(addr: UInt, size: UInt, device: Seq[String]): Bool = {
    device.map(check_in_range(addr, size, _)).reduce(_ || _)
  }
}

trait BranchConfig {
  val BTBSetWidth    = 5
  val BTBSet         = pow(2, BTBSetWidth).toInt
  val BTBWay         = 2 // support 1 or 2
  val BTBTagWidth    = 11
  val BTBBtaComWidth = 12 // when way > 1, the width of the common part of bta
  val br_type_id     = Map("call" -> 0, "return" -> 1, "taken_br" -> 2, "typeb" -> 3, "not_br" -> 4, "intr" -> 5)

  val PhtAddrWidth = 5
  val PhtDepth     = pow(2, PhtAddrWidth).toInt

  val RASDepth  = 6
  val RASCntNum = 4
}

trait ALU_op {
  //  val sNone :: sOne1 :: sTwo1s :: Nil = Enum(3)
  val op_x = 0.U
  // + -
  val op_add = 1.U // 00001
  val op_sub = 3.U // 00011
  // shift
  val op_sll = 2.U // 00010 left_shift
  val op_srl = 4.U // 00100 right_shift_logical
  val op_sra = 5.U // 00101 right_shift_arithmetic
  // shift w
  val op_srlw = 9.U // 01001
  val op_sraw = 10.U // 01010
  val op_sllw = 11.U // 01011
  // mul
  val op_mul    = 8.U // 01000
  val op_mulw   = 12.U // 01100
  val op_mulh   = 13.U // 01101
  val op_mulhs  = 14.U // 01110
  val op_mulhsu = 15.U // 01111
  //  div
  val op_div   = 16.U // 10000
  val op_divs  = 17.U // 10001
  val op_divsw = 18.U // 10010
  val op_divw  = 19.U // 10011
  // rem
  val op_rem   = 20.U // 10100
  val op_rems  = 21.U // 10101
  val op_remsw = 22.U // 10110
  val op_remw  = 23.U // 10111
  // logic
  val op_xor = 6.U // 00110
  val op_or  = 7.U // 00111 csr
  val op_and = 24.U // 11000
  // csr
  val op_andinv = 25.U // 11001

}

trait Judge_op {
  //    val jop_x :: sOne1 :: sTwo1s :: Nil = Enum(3)

  val jop_x = 0.U
  // B
  val jop_beq  = 1.U
  val jop_bne  = 3.U
  val jop_blt  = 2.U
  val jop_bltu = 6.U
  val jop_bge  = 7.U
  val jop_bgeu = 5.U
  // set
  val jop_slt  = 4.U
  val jop_sltu = 12.U
  // sext
  val jop_sextw = 13.U // 截取32位做符号位拓展
  // val jop_sexthw = 7.U
  // val jop_sextb  = 8.U
  // uext
  // val jop_uextw  = 9.U
  // val jop_uexthw = 10.U
  // val jop_uextb  = 11.U

}

trait Judge_Load_op {
  //    val jop_x :: sOne1 :: sTwo1s :: Nil = Enum(3)

  val jlop_x = 0.U
  // sext
  val jlop_sextw  = 1.U
  val jlop_sexthw = 2.U
  val jlop_sextb  = 3.U
  // uext
  val jlop_uextw  = 4.U
  val jlop_uexthw = 5.U
  val jlop_uextb  = 6.U

}

trait Csr_op {
  //    val jop_x :: sOne1 :: sTwo1s :: Nil = Enum(3)

  val csr_x = 0.U

  val csr_rw  = 2.U
  val csr_rwi = 3.U

  val csr_rs  = 4.U
  val csr_rsi = 5.U

  val csr_rc  = 6.U
  val csr_rci = 7.U

}

trait IDU_op extends ALU_op with Judge_op with Csr_op with Judge_Load_op

trait ext_function {
  def sext_64(inst_p: UInt): UInt = {
    Cat(sext(inst_p, inst_p.getWidth), inst_p)
  }

  // 取部分指令inst_p,将最高位符号位扩展，扩展的位数根据低比特有效值有多少位valid_bit决定
  // 返回扩展出来的部分
  //sext(io.inst(31, 20), 12) 代表inst信号的部分中31bit为符号位，返回52bit(64-12)的全0或全1
  def sext(inst_p: UInt, valid_bit: Int, left_shift: Int = 0): UInt =
    VecInit(Seq.fill(64 - valid_bit - left_shift)(inst_p(inst_p.getWidth - 1))).asUInt

  def uext_64(inst_p: UInt): UInt = {
    Cat(uext(inst_p, inst_p.getWidth), inst_p)
  }

  def uext(inst_p: UInt, valid_bit: Int, left_shift: Int = 0): UInt =
    VecInit(Seq.fill(64 - valid_bit - left_shift)(0.B)).asUInt
}

trait util_function {
  def riseEdge(in: Bool): Bool = {
    !RegNext(in, 0.B) && in
  }

  def fallEdge(in: Bool): Bool = {
    RegNext(in, 0.B) && !in
  }

  def dualEdge(in: Bool): Bool = {
    RegNext(in, 0.B) ^ in
  }

  def rangeAdd(cnt: UInt, num: Int): UInt = {
    Mux(cnt === (num - 1).U, 0.U, cnt + 1.U)
  }

  def rangeSub(cnt: UInt, num: Int): UInt = {
    Mux(cnt === 0.U, (num - 1).U, cnt - 1.U)
  }

}

object Assert extends RVNoobModeConfig {

  class DpiAssert extends BlackBox {
    val io = IO(new Bundle {
      val clock  = Input(Clock())
      val reset  = Input(Reset())
      val en = Input(Bool())
    })

  }

  object DpiAssert {
    def apply(
      clock:  Clock,
      reset:  Reset,
      en: Bool
    ): DpiAssert = {
      val u_assert = Module(new DpiAssert)
      u_assert.io.clock  <> clock
      u_assert.io.reset  <> reset
      u_assert.io.en <> en
      u_assert
    }
  }

  def apply(clock: Clock, reset: Reset, cond: Bool, message: String, data: Bits*): Any = {
    DpiAssert(clock, reset, !cond)
    when(!cond) {
//      printf(message, data)
    }
  }

}
