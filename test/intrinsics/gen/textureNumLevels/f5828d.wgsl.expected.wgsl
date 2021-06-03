[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn textureNumLevels_f5828d() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_f5828d();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_f5828d();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_f5828d();
}
