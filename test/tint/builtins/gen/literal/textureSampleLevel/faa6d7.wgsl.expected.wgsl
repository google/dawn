@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_faa6d7() {
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, vec2<f32>(), 1u, 1.0f, vec2<i32>());
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
