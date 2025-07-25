@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16unorm, read_write>;

fn textureStore_1eb1a6() {
  textureStore(arg_0, vec3<u32>(1u), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_1eb1a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_1eb1a6();
}
