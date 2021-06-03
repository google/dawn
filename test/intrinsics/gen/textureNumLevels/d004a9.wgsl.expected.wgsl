[[group(1), binding(0)]] var arg_0 : texture_2d_array<i32>;

fn textureNumLevels_d004a9() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_d004a9();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_d004a9();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_d004a9();
}
