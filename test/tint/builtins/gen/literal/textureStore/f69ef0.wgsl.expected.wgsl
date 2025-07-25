@group(1) @binding(0) var arg_0 : texture_storage_2d<r8sint, read_write>;

fn textureStore_f69ef0() {
  textureStore(arg_0, vec2<u32>(1u), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_f69ef0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_f69ef0();
}
