@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32sint, write>;

fn textureStore_8bb287() {
  textureStore(arg_0, vec2<u32>(1u), 1u, vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_8bb287();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_8bb287();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8bb287();
}
