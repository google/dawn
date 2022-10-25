@group(1) @binding(1) var arg_1 : texture_cube<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_32c4e8() {
  const arg_0 = 1i;
  var arg_3 = vec3<f32>();
  var res : vec4<f32> = textureGather(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_32c4e8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_32c4e8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_32c4e8();
}
