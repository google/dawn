[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba16sint>;

fn textureStore_5a2f8f() {
  textureStore(arg_0, 1, vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_5a2f8f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_5a2f8f();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_5a2f8f();
}
