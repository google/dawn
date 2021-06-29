[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rgba32sint, write>;

fn textureNumLayers_13b4ce() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLayers_13b4ce();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_13b4ce();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLayers_13b4ce();
}
