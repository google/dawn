@group(1) @binding(0) var arg_0 : texture_storage_1d<rg16uint, read_write>;

fn textureStore_a0b75d() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_a0b75d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a0b75d();
}
