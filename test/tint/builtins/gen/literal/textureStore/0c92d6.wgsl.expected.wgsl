@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r16sint, write>;

fn textureStore_0c92d6() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_0c92d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_0c92d6();
}
