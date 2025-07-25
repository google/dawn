@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16unorm, read_write>;

fn textureStore_df17b1() {
  textureStore(arg_0, vec3<i32>(1i), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_df17b1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_df17b1();
}
