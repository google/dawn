[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba32uint>;

fn textureStore_fb9a8f() {
  textureStore(arg_0, 1, vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_fb9a8f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_fb9a8f();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_fb9a8f();
}
