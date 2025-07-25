@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r16uint, write>;

fn textureStore_e8d4d9() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_e8d4d9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_e8d4d9();
}
