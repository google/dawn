@group(1) @binding(0) var arg_0 : texture_storage_2d_array<bgra8unorm, write>;

fn textureStore_27063a() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_27063a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_27063a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_27063a();
}
