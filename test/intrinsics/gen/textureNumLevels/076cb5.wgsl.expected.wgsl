[[group(1), binding(0)]] var arg_0 : texture_depth_cube;

fn textureNumLevels_076cb5() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_076cb5();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_076cb5();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_076cb5();
}
