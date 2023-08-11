enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba32uint, read_write>;

fn textureStore_6d259f() {
  textureStore(arg_0, vec2<i32>(1i), vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_6d259f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_6d259f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6d259f();
}
