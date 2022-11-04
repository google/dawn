@group(1) @binding(0) var arg_0 : texture_storage_2d<r32float, write>;

fn textureStore_0af6b5() {
  textureStore(arg_0, vec2<i32>(1i), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_0af6b5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_0af6b5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_0af6b5();
}
