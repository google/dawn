@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgb10a2uint, read_write>;

fn textureStore_a91fe3() {
  textureStore(arg_0, vec2<u32>(1u), 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_a91fe3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a91fe3();
}
