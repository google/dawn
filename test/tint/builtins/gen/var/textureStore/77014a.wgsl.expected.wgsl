@group(1) @binding(0) var arg_0 : texture_storage_2d<rg8uint, read_write>;

fn textureStore_77014a() {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_77014a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_77014a();
}
