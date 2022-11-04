@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32float, write>;

fn textureStore_df2ca4() {
  textureStore(arg_0, vec2<i32>(1i), 1u, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_df2ca4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_df2ca4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_df2ca4();
}
