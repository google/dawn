@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16snorm, write>;

fn textureStore_72f1d3() {
  textureStore(arg_0, vec3<i32>(1i), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_72f1d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_72f1d3();
}
