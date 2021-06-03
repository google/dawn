[[group(1), binding(0)]] var arg_0 : texture_2d_array<u32>;

fn textureNumLevels_5101cf() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_5101cf();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_5101cf();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_5101cf();
}
