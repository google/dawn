@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16unorm, read_write>;

fn textureStore_2e9685() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_2e9685();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_2e9685();
}
