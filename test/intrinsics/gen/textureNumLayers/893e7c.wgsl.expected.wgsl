[[group(1), binding(0)]] var arg_0 : texture_2d_array<i32>;

fn textureNumLayers_893e7c() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLayers_893e7c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_893e7c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLayers_893e7c();
}
