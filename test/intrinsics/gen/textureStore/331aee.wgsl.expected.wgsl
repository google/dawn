[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba32float>;

fn textureStore_331aee() {
  textureStore(arg_0, vec3<i32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_331aee();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_331aee();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_331aee();
}
