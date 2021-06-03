[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba32float>;

fn textureStore_05ce15() {
  textureStore(arg_0, vec2<i32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_05ce15();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_05ce15();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_05ce15();
}
