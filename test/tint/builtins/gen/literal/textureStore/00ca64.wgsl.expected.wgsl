@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32uint, write>;

fn textureStore_00ca64() {
  textureStore(arg_0, vec2<i32>(), 1u, vec4<u32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_00ca64();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_00ca64();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_00ca64();
}
