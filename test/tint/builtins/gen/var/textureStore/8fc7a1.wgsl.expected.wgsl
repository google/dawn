@group(1) @binding(0) var arg_0 : texture_storage_2d<r16sint, read_write>;

fn textureStore_8fc7a1() {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_8fc7a1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8fc7a1();
}
