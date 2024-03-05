package RVNoob
import circt.stage._

object RVNoobCoreGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/soc"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new RVNoobCore), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}

object RVNoobGen extends App {
  (new ChiselStage)
    .execute(
      args,
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new RVNoob()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}
