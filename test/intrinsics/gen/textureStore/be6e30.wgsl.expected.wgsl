[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba16float>;

fn textureStore_be6e30() {
  textureStore(arg_0, vec2<i32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_be6e30();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_be6e30();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_be6e30();
}
