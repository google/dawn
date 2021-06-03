[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba8snorm>;

fn textureStore_dd7d81() {
  textureStore(arg_0, vec3<i32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_dd7d81();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_dd7d81();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_dd7d81();
}
