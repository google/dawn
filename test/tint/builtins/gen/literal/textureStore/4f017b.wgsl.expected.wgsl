@group(1) @binding(0) var arg_0 : texture_storage_1d<rg8unorm, write>;

fn textureStore_4f017b() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_4f017b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_4f017b();
}
