@group(1) @binding(0) var arg_0 : texture_storage_1d<rg8unorm, write>;

fn textureStore_94e54c() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_94e54c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_94e54c();
}
