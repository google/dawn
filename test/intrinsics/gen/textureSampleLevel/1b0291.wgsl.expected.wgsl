[[group(1), binding(0)]] var arg_0 : texture_depth_cube;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSampleLevel_1b0291() {
  var res : f32 = textureSampleLevel(arg_0, arg_1, vec3<f32>(), 0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureSampleLevel_1b0291();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureSampleLevel_1b0291();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureSampleLevel_1b0291();
}
