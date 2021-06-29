[[group(1), binding(0)]] var arg_0 : texture_depth_2d;

fn textureNumLevels_b1b12b() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_b1b12b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_b1b12b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_b1b12b();
}
