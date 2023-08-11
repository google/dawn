enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba16float, read_write>;

fn textureStore_8a46ff() {
  textureStore(arg_0, vec2<i32>(1i), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_8a46ff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_8a46ff();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8a46ff();
}
