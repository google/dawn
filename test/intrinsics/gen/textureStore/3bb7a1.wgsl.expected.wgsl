[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<r32float>;

fn textureStore_3bb7a1() {
  textureStore(arg_0, vec2<i32>(), 1, vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_3bb7a1();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_3bb7a1();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_3bb7a1();
}
