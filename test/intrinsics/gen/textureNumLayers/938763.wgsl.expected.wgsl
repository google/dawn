[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rg32uint>;

fn textureNumLayers_938763() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_938763();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_938763();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_938763();
}
