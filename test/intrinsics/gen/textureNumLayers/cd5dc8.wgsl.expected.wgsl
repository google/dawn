[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba32uint>;

fn textureNumLayers_cd5dc8() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_cd5dc8();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_cd5dc8();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_cd5dc8();
}
