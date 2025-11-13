requires fragment_depth;

struct FragDepthAnyOutput {
  @builtin(frag_depth)
  frag_depth : f32,
}

struct FragDepthLessOutput {
  @builtin(frag_depth)
  frag_depth : f32,
}

struct FragDepthGreaterOutput {
  @builtin(frag_depth)
  frag_depth : f32,
}

@fragment
fn any() -> FragDepthAnyOutput {
  return FragDepthAnyOutput(1.0);
}

@fragment
fn less() -> FragDepthLessOutput {
  return FragDepthLessOutput(1.0);
}

@fragment
fn greater() -> FragDepthGreaterOutput {
  return FragDepthGreaterOutput(1.0);
}
