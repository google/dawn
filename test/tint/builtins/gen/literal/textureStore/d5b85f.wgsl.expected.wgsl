@group(1) @binding(0) var arg_0 : texture_storage_2d<r16uint, write>;

fn textureStore_d5b85f() {
  textureStore(arg_0, vec2<i32>(1i), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_d5b85f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d5b85f();
}
