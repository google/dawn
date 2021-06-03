[[group(1), binding(0)]] var arg_0 : texture_cube<u32>;

fn textureNumLevels_ed078b() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_ed078b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_ed078b();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_ed078b();
}
