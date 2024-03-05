package RVNoob.MulDiv
import circt.stage._

object BoothShiftMultiplierGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/test"),
      Seq(
        chisel3.stage.ChiselGeneratorAnnotation(() => new BoothShiftMultiplier()),
        CIRCTTargetAnnotation(CIRCTTarget.Verilog)
      )
    )
}

object ShiftDividerGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/test"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new ShiftDivider()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}
