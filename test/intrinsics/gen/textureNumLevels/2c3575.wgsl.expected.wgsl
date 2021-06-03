[[group(1), binding(0)]] var arg_0 : texture_depth_cube_array;

fn textureNumLevels_2c3575() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_2c3575();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_2c3575();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_2c3575();
}
