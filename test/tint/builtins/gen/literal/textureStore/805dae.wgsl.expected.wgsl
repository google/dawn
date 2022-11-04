@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8snorm, write>;

fn textureStore_805dae() {
  textureStore(arg_0, vec2<i32>(1i), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_805dae();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_805dae();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_805dae();
}
