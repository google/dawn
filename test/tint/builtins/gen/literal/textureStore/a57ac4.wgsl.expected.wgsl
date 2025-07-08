@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg8snorm, read_write>;

fn textureStore_a57ac4() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_a57ac4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a57ac4();
}
