[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba8unorm>;

fn textureStore_7f7fae() {
  textureStore(arg_0, 1, vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_7f7fae();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_7f7fae();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_7f7fae();
}
