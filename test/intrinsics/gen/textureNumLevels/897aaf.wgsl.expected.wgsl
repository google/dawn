[[group(1), binding(0)]] var arg_0 : texture_cube<f32>;

fn textureNumLevels_897aaf() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_897aaf();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_897aaf();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_897aaf();
}
