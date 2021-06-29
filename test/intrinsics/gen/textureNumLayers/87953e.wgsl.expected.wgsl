[[group(1), binding(0)]] var arg_0 : texture_2d_array<u32>;

fn textureNumLayers_87953e() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLayers_87953e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_87953e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLayers_87953e();
}
