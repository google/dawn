enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16float, read_write>;

fn textureStore_90960e() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_90960e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_90960e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_90960e();
}
