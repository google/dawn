[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba8unorm>;

fn textureNumLayers_45155d() {
  var res : i32 = textureNumLayers(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureNumLayers_45155d();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLayers_45155d();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLayers_45155d();
}
