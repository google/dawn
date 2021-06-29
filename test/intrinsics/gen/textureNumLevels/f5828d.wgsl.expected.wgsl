[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn textureNumLevels_f5828d() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_f5828d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_f5828d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_f5828d();
}
