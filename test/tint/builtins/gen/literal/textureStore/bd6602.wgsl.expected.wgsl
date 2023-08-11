enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg32uint, read_write>;

fn textureStore_bd6602() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_bd6602();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_bd6602();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_bd6602();
}
