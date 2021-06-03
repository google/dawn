[[group(1), binding(0)]] var arg_0 : texture_multisampled_2d<u32>;

fn textureNumSamples_42f8bb() {
  var res : i32 = textureNumSamples(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumSamples_42f8bb();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumSamples_42f8bb();
}

[[stage(compute)]]
fn compute_main() {
  textureNumSamples_42f8bb();
}
