[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rg32sint>;

fn textureStore_31745b() {
  textureStore(arg_0, vec2<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_31745b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_31745b();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_31745b();
}
