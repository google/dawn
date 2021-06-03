[[group(1), binding(0)]] var arg_0 : texture_multisampled_2d<i32>;

fn textureNumSamples_449d23() {
  var res : i32 = textureNumSamples(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumSamples_449d23();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumSamples_449d23();
}

[[stage(compute)]]
fn compute_main() {
  textureNumSamples_449d23();
}
