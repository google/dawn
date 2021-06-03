[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba32sint>;

fn textureStore_7cec8d() {
  textureStore(arg_0, vec2<i32>(), 1, vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_7cec8d();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_7cec8d();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_7cec8d();
}
