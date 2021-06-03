[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba16float>;

fn textureStore_32f368() {
  textureStore(arg_0, vec2<i32>(), 1, vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_32f368();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_32f368();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_32f368();
}
