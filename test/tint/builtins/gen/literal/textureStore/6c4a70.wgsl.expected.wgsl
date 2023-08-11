enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d<r32sint, read_write>;

fn textureStore_6c4a70() {
  textureStore(arg_0, vec2<u32>(1u), vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_6c4a70();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_6c4a70();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6c4a70();
}
