@group(1) @binding(0) var arg_0 : texture_storage_1d<rg16float, write>;

fn textureStore_4ec7e0() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_4ec7e0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_4ec7e0();
}
