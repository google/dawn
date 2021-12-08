[[group(1), binding(0)]] var arg_0 : texture_depth_cube;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureGather_10c554() {
  var res : vec4<f32> = textureGather(arg_0, arg_1, vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureGather_10c554();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureGather_10c554();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureGather_10c554();
}
