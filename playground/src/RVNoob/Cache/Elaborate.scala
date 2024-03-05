package RVNoob.Cache
import circt.stage._

object DCacheGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/test"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new DCache()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}
