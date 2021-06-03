[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

fn textureNumLayers_024820() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_024820();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_024820();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_024820();
}
