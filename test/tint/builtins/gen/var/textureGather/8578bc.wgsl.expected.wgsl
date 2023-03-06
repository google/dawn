@group(1) @binding(1) var arg_1 : texture_cube_array<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_8578bc() {
  const arg_0 = 1u;
  var arg_3 = vec3<f32>(1.0f);
  var arg_4 = 1u;
  var res : vec4<f32> = textureGather(arg_0, arg_1, arg_2, arg_3, arg_4);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_8578bc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_8578bc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_8578bc();
}
