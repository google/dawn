[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba8unorm, write>;

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

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureStore_7f7fae();
}
