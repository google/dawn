@group(1) @binding(0) var arg_0 : texture_storage_2d<r8uint, read_write>;

fn textureStore_7eb30e() {
  textureStore(arg_0, vec2<u32>(1u), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_7eb30e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_7eb30e();
}
