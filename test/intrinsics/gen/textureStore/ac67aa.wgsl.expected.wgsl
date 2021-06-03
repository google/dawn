[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rg32uint>;

fn textureStore_ac67aa() {
  textureStore(arg_0, vec3<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_ac67aa();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_ac67aa();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_ac67aa();
}
