@group(1) @binding(0) var arg_0 : texture_storage_2d<rgb10a2uint, write>;

fn textureStore_510b7d() {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_510b7d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_510b7d();
}
