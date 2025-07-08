@group(1) @binding(0) var arg_0 : texture_storage_1d<r16float, write>;

fn textureStore_ed8eb5() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_ed8eb5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_ed8eb5();
}
