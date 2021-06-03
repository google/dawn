[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba32float>;

fn textureStore_6b75c3() {
  textureStore(arg_0, 1, vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_6b75c3();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_6b75c3();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_6b75c3();
}
