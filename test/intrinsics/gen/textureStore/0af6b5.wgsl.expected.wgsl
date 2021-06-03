[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<r32float>;

fn textureStore_0af6b5() {
  textureStore(arg_0, vec2<i32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_0af6b5();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_0af6b5();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_0af6b5();
}
