[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba16uint>;

fn textureStore_068641() {
  textureStore(arg_0, vec3<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_068641();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_068641();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_068641();
}
