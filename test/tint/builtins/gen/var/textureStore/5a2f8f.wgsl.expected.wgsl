@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16sint, write>;

fn textureStore_5a2f8f() {
  var arg_1 = 1i;
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_5a2f8f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_5a2f8f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5a2f8f();
}
