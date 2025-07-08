@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r16float, write>;

fn textureStore_ad6e3b() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_ad6e3b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_ad6e3b();
}
