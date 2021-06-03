[[group(1), binding(0)]] var arg_0 : texture_depth_2d;

fn textureNumLevels_b1b12b() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_b1b12b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_b1b12b();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_b1b12b();
}
