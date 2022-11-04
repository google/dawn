@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8uint, write>;

fn textureStore_0a1a79() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_0a1a79();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_0a1a79();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_0a1a79();
}
