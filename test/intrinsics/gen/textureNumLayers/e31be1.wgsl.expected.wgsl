[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba8snorm>;

fn textureNumLayers_e31be1() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_e31be1();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_e31be1();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_e31be1();
}
