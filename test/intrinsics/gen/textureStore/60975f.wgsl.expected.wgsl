[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba8unorm>;

fn textureStore_60975f() {
  textureStore(arg_0, vec2<i32>(), 1, vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_60975f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_60975f();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_60975f();
}
