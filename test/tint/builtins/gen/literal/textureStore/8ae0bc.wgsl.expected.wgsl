enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8snorm, read_write>;

fn textureStore_8ae0bc() {
  textureStore(arg_0, vec2<i32>(1i), 1u, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_8ae0bc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_8ae0bc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8ae0bc();
}
