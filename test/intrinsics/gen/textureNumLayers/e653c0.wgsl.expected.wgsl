[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn textureNumLayers_e653c0() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_e653c0();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_e653c0();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_e653c0();
}
