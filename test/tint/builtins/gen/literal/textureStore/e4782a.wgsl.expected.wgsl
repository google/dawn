@group(1) @binding(0) var arg_0 : texture_storage_1d<r8sint, read_write>;

fn textureStore_e4782a() {
  textureStore(arg_0, 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_e4782a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_e4782a();
}
