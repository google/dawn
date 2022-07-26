@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_b7c55c() {
  var arg_2 = vec2<f32>();
  var arg_3 = 1;
  var arg_4 = 1.0f;
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, arg_2, arg_3, arg_4, vec2<i32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_b7c55c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_b7c55c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_b7c55c();
}
