[[group(1), binding(0)]] var arg_0 : texture_cube_array<i32>;

fn textureNumLevels_a91c03() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_a91c03();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_a91c03();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_a91c03();
}
