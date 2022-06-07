@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureNumSamples_a3c8a0() {
  var res : i32 = textureNumSamples(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumSamples_a3c8a0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumSamples_a3c8a0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumSamples_a3c8a0();
}
