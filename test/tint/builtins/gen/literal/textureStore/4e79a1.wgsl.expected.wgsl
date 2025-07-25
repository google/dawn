@group(1) @binding(0) var arg_0 : texture_storage_1d<rg16snorm, write>;

fn textureStore_4e79a1() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_4e79a1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_4e79a1();
}
