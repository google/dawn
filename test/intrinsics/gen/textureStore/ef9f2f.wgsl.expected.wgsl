[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<r32uint>;

fn textureStore_ef9f2f() {
  textureStore(arg_0, vec3<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_ef9f2f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_ef9f2f();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_ef9f2f();
}
