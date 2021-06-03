[[group(1), binding(0)]] var arg_0 : texture_2d<f32>;

fn textureNumLevels_e67231() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_e67231();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_e67231();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_e67231();
}
