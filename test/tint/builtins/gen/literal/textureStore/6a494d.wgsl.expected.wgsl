@group(1) @binding(0) var arg_0 : texture_storage_2d<r8sint, read_write>;

fn textureStore_6a494d() {
  textureStore(arg_0, vec2<i32>(1i), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_6a494d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6a494d();
}
