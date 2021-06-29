[[group(1), binding(0)]] var arg_0 : texture_cube_array<f32>;

fn textureNumLevels_aee7c8() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_aee7c8();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_aee7c8();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_aee7c8();
}
