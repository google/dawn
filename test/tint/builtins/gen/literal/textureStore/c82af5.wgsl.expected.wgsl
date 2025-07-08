@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r8sint, write>;

fn textureStore_c82af5() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_c82af5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c82af5();
}
