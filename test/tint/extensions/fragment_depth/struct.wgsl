requires fragment_depth;

struct FragDepthLessOutput {
  @builtin(frag_depth, less) frag_depth : f32,
};

struct FragDepthGreaterOutput {
  @builtin(frag_depth, greater) frag_depth : f32,
};

@fragment
fn less() -> FragDepthLessOutput { return FragDepthLessOutput(1.0); }

@fragment
fn greater() -> FragDepthGreaterOutput { return FragDepthGreaterOutput(1.0); }
