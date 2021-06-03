[[group(1), binding(0)]] var arg_0 : texture_multisampled_2d<f32>;

fn textureNumSamples_2c6f14() {
  var res : i32 = textureNumSamples(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumSamples_2c6f14();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumSamples_2c6f14();
}

[[stage(compute)]]
fn compute_main() {
  textureNumSamples_2c6f14();
}
