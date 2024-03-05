package RVNoob.Pipeline

import circt.stage._

object PipelineCtrlGen extends App {
  (new ChiselStage)
    .execute(
      Array("--target-dir", "./build/test"),
      Seq(chisel3.stage.ChiselGeneratorAnnotation(() => new PipelineCtrl()),  CIRCTTargetAnnotation(CIRCTTarget.Verilog))
    )
}