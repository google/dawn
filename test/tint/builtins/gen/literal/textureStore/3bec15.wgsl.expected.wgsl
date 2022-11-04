@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8uint, write>;

fn textureStore_3bec15() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_3bec15();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_3bec15();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_3bec15();
}
