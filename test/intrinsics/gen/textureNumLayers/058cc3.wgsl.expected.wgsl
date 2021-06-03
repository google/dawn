[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rg32sint>;

fn textureNumLayers_058cc3() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_058cc3();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_058cc3();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_058cc3();
}
