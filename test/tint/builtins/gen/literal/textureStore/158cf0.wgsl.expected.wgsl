@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32uint, write>;

fn textureStore_158cf0() {
  textureStore(arg_0, vec2<u32>(1u), 1u, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_158cf0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_158cf0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_158cf0();
}
