[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba32float>;

fn textureNumLayers_68a65b() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLayers_68a65b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_68a65b();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_68a65b();
}
