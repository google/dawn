enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba32uint, read_write>;

fn textureStore_2173fd() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_2173fd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_2173fd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_2173fd();
}
