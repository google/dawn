@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg8snorm, write>;

fn textureStore_a48681() {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = 1i;
  var arg_3 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2, arg_3);
}

@fragment
fn fragment_main() {
  textureStore_a48681();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a48681();
}
