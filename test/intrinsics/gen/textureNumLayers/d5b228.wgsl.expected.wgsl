[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<r32float>;

fn textureNumLayers_d5b228() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_d5b228();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_d5b228();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_d5b228();
}
