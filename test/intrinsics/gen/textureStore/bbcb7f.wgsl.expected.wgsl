[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba8sint>;

fn textureStore_bbcb7f() {
  textureStore(arg_0, vec2<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_bbcb7f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_bbcb7f();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_bbcb7f();
}
