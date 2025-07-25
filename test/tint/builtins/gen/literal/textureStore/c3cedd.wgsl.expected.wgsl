@group(1) @binding(0) var arg_0 : texture_storage_2d<rg16unorm, read_write>;

fn textureStore_c3cedd() {
  textureStore(arg_0, vec2<u32>(1u), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_c3cedd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c3cedd();
}
