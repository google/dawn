@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8uint, read_write>;

fn textureStore_d0d62c() {
  textureStore(arg_0, vec2<i32>(1i), vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_d0d62c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_d0d62c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d0d62c();
}
