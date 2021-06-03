[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba16sint>;

fn textureStore_9e3ec5() {
  textureStore(arg_0, vec2<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_9e3ec5();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_9e3ec5();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_9e3ec5();
}
