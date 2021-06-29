[[group(1), binding(0)]] var arg_0 : texture_cube_array<i32>;

fn textureNumLayers_85f980() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLayers_85f980();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_85f980();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLayers_85f980();
}
