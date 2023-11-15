enable chromium_internal_dual_source_blending;

struct FragInput {
  @location(0) a : vec4<f32>,
  @location(1) b : vec4<f32>,
};

struct FragOutput {
  @location(0) @index(0) color : vec4<f32>,
  @location(0) @index(1) blend : vec4<f32>,
};

@fragment
fn frag_main(in : FragInput) -> FragOutput {
  var output : FragOutput;
  output.color = in.a;
  output.blend = in.b;
  return output;
}
