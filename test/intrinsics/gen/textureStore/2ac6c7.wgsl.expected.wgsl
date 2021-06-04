[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<r32float>;

fn textureStore_2ac6c7() {
  textureStore(arg_0, 1, vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_2ac6c7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_2ac6c7();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_2ac6c7();
}
