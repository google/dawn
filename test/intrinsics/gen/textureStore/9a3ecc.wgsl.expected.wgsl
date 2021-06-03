[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba32sint>;

fn textureStore_9a3ecc() {
  textureStore(arg_0, vec3<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_9a3ecc();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_9a3ecc();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_9a3ecc();
}
