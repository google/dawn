@group(1) @binding(0) var arg_0 : texture_storage_3d<r16snorm, write>;

fn textureStore_b88f1e() {
  textureStore(arg_0, vec3<i32>(1i), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_b88f1e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_b88f1e();
}
