[[group(1), binding(0)]] var arg_0 : texture_depth_cube;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSampleLevel_1b0291() {
  var res : f32 = textureSampleLevel(arg_0, arg_1, vec3<f32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureSampleLevel_1b0291();
}

[[stage(fragment)]]
fn fragment_main() {
  textureSampleLevel_1b0291();
}

[[stage(compute)]]
fn compute_main() {
  textureSampleLevel_1b0291();
}
