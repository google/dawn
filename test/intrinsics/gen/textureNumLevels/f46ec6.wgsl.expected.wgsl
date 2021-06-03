[[group(1), binding(0)]] var arg_0 : texture_cube_array<u32>;

fn textureNumLevels_f46ec6() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_f46ec6();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_f46ec6();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_f46ec6();
}
