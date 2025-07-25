@group(1) @binding(0) var arg_0 : texture_storage_1d<rg16snorm, read_write>;

fn textureStore_3eda2c() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_3eda2c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_3eda2c();
}
