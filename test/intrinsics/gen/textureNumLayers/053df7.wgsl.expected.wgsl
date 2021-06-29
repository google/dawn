[[group(1), binding(0)]] var arg_0 : texture_cube_array<u32>;

fn textureNumLayers_053df7() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLayers_053df7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_053df7();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLayers_053df7();
}
