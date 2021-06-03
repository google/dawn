[[group(1), binding(0)]] var arg_0 : texture_cube_array<u32>;

fn textureNumLayers_053df7() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_053df7();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_053df7();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_053df7();
}
