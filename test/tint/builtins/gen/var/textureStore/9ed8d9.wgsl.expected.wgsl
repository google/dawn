@group(1) @binding(0) var arg_0 : texture_storage_1d<rg8snorm, read_write>;

fn textureStore_9ed8d9() {
  var arg_1 = 1i;
  var arg_2 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_9ed8d9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9ed8d9();
}
