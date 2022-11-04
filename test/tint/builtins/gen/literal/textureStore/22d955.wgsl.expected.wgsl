@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8uint, write>;

fn textureStore_22d955() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_22d955();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_22d955();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_22d955();
}
