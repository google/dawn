[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<r32float>;

fn textureNumLayers_be3acb() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_be3acb();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_be3acb();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_be3acb();
}
