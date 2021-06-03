[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

fn textureNumLevels_105988() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_105988();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_105988();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_105988();
}
