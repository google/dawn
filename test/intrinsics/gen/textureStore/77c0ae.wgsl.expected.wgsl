[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba8uint>;

fn textureStore_77c0ae() {
  textureStore(arg_0, vec2<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_77c0ae();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_77c0ae();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_77c0ae();
}
