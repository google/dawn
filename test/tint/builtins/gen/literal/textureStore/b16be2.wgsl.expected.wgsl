@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16snorm, read_write>;

fn textureStore_b16be2() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_b16be2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_b16be2();
}
