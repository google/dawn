@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r16uint, write>;

fn textureStore_6d7562() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_6d7562();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6d7562();
}
