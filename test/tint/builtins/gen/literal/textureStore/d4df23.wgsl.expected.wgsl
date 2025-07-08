@group(1) @binding(0) var arg_0 : texture_storage_2d<rg8uint, write>;

fn textureStore_d4df23() {
  textureStore(arg_0, vec2<u32>(1u), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_d4df23();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d4df23();
}
