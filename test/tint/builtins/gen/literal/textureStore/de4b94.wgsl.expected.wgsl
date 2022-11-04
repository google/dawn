@group(1) @binding(0) var arg_0 : texture_storage_1d<r32sint, write>;

fn textureStore_de4b94() {
  textureStore(arg_0, 1u, vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_de4b94();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_de4b94();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_de4b94();
}
