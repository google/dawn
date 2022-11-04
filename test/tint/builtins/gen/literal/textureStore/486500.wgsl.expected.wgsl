@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba32sint, write>;

fn textureStore_486500() {
  textureStore(arg_0, vec3<u32>(1u), vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_486500();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_486500();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_486500();
}
