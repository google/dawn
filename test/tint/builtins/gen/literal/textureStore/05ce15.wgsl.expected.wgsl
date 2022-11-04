@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba32float, write>;

fn textureStore_05ce15() {
  textureStore(arg_0, vec2<i32>(1i), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_05ce15();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_05ce15();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_05ce15();
}
