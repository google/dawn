[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba32sint>;

fn textureNumLayers_13b4ce() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_13b4ce();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_13b4ce();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_13b4ce();
}
