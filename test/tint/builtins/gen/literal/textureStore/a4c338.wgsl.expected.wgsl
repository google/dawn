@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16float, write>;

fn textureStore_a4c338() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_a4c338();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_a4c338();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a4c338();
}
