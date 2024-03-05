package RVNoob.Core
import circt.stage._

object RVNoobCoreGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/soc"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new RVNoobCore), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}

object RVNoobTileGen extends App {
  (new ChiselStage)
    .execute(
      args,
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new RVNoobTile()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}

object RVNoobSimGen extends App {
  (new ChiselStage)
    .execute(
      args,
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new RVNoobSim()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}
