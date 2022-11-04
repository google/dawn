@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32sint, write>;

fn textureStore_28a7ec() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_28a7ec();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_28a7ec();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_28a7ec();
}
