@group(1) @binding(0) var arg_0 : texture_storage_1d<rg8snorm, write>;

fn textureStore_920534() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_920534();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_920534();
}
