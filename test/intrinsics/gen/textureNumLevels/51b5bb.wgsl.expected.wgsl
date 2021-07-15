[[group(1), binding(0)]] var arg_0 : texture_1d<f32>;

fn textureNumLevels_51b5bb() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_51b5bb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_51b5bb();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_51b5bb();
}
