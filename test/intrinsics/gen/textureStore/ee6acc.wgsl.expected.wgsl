[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rg32float>;

fn textureStore_ee6acc() {
  textureStore(arg_0, vec3<i32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_ee6acc();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_ee6acc();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_ee6acc();
}
