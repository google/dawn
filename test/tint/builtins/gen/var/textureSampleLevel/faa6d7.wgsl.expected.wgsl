@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_faa6d7() {
  var arg_2 = vec2<f32>();
  var arg_3 = 1u;
  var arg_4 = 1.0f;
  const arg_5 = vec2<i32>();
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_faa6d7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_faa6d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_faa6d7();
}
