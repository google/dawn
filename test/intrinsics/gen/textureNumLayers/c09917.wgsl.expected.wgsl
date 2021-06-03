[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba32uint>;

fn textureNumLayers_c09917() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_c09917();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_c09917();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_c09917();
}
