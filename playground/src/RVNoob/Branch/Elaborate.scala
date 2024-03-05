package RVNoob.Branch
import circt.stage._

object RASGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/test"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new RAS()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}

object BTBGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/test"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new BTB()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}

object PHTsGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/test"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new PHTs()), CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}