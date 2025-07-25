@group(1) @binding(0) var arg_0 : texture_storage_1d<r16unorm, write>;

fn textureStore_948ce8() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_948ce8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_948ce8();
}
