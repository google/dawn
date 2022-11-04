@group(1) @binding(0) var arg_0 : texture_2d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleGrad_d4e3c5() {
  var res : vec4<f32> = textureSampleGrad(arg_0, arg_1, vec2<f32>(1.0f), vec2<f32>(1.0f), vec2<f32>(1.0f), vec2<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleGrad_d4e3c5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleGrad_d4e3c5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleGrad_d4e3c5();
}
