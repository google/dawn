@group(1) @binding(0) var arg_0 : texture_storage_1d<r16sint, read_write>;

fn textureStore_5b4f29() {
  var arg_1 = 1u;
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_5b4f29();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5b4f29();
}
