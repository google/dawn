@group(1) @binding(0) var arg_0 : texture_storage_1d<r16uint, read_write>;

fn textureStore_40eba3() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_40eba3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_40eba3();
}
