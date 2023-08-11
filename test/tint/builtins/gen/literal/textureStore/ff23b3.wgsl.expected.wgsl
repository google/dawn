enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16uint, read_write>;

fn textureStore_ff23b3() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_ff23b3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_ff23b3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_ff23b3();
}
