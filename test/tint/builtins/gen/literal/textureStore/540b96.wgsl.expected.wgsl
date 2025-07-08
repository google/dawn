@group(1) @binding(0) var arg_0 : texture_storage_1d<rg8uint, write>;

fn textureStore_540b96() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_540b96();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_540b96();
}
