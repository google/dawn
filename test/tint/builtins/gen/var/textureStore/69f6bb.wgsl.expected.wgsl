@group(1) @binding(0) var arg_0 : texture_storage_3d<r8sint, write>;

fn textureStore_69f6bb() {
  var arg_1 = vec3<i32>(1i);
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_69f6bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_69f6bb();
}
