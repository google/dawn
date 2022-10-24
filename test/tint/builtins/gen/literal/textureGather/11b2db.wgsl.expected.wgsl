@group(1) @binding(1) var arg_1 : texture_cube<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_11b2db() {
  var res : vec4<f32> = textureGather(1u, arg_1, arg_2, vec3<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_11b2db();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_11b2db();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_11b2db();
}
