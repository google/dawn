[[group(1), binding(0)]] var arg_0 : texture_cube_array<i32>;

fn textureNumLevels_a91c03() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLevels_a91c03();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_a91c03();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_a91c03();
}
