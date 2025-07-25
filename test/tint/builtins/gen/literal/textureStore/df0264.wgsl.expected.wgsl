@group(1) @binding(0) var arg_0 : texture_storage_1d<r16snorm, read_write>;

fn textureStore_df0264() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_df0264();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_df0264();
}
