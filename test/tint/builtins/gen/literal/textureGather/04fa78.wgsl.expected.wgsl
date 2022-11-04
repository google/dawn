@group(1) @binding(1) var arg_1 : texture_cube_array<i32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_04fa78() {
  var res : vec4<i32> = textureGather(1u, arg_1, arg_2, vec3<f32>(1.0f), 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_04fa78();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_04fa78();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_04fa78();
}
