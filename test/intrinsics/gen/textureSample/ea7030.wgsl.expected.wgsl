[[group(1), binding(0)]] var arg_0 : texture_depth_cube;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSample_ea7030() {
  var res : f32 = textureSample(arg_0, arg_1, vec3<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  textureSample_ea7030();
}
