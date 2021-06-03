[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<r32sint>;

fn textureNumLayers_22e53b() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_22e53b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_22e53b();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_22e53b();
}
