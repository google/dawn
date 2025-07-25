@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba16snorm, read_write>;

fn textureStore_ef1ccd() {
  textureStore(arg_0, vec2<i32>(1i), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_ef1ccd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_ef1ccd();
}
