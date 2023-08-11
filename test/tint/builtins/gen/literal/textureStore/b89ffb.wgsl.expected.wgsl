enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<bgra8unorm, read_write>;

fn textureStore_b89ffb() {
  textureStore(arg_0, vec2<i32>(1i), 1u, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_b89ffb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_b89ffb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_b89ffb();
}
