@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg8snorm, write>;

fn textureStore_9137a1() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_9137a1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9137a1();
}
