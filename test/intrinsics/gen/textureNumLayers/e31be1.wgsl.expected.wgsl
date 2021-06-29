[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rgba8snorm, write>;

fn textureNumLayers_e31be1() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLayers_e31be1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_e31be1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLayers_e31be1();
}
