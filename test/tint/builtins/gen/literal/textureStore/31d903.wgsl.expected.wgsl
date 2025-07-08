@group(1) @binding(0) var arg_0 : texture_storage_1d<rg16float, read_write>;

fn textureStore_31d903() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_31d903();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_31d903();
}
