@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r16uint, read_write>;

fn textureStore_1b763e() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_1b763e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_1b763e();
}
