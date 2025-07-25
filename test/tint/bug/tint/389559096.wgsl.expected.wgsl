struct VSInput {
  @location(0)
  val : vec4f,
}

struct VSOutputs {
  @location(0) @interpolate(flat, either)
  result : i32,
  @builtin(position)
  position : vec4f,
}

@vertex
fn vsMain(vtxIn : VSInput) -> VSOutputs {
  return VSOutputs(1, vtxIn.val);
}
