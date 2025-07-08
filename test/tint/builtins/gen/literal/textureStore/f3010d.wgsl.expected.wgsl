@group(1) @binding(0) var arg_0 : texture_storage_1d<r8snorm, read_write>;

fn textureStore_f3010d() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_f3010d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_f3010d();
}
