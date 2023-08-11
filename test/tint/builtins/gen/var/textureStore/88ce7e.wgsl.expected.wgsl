enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8sint, read_write>;

fn textureStore_88ce7e() {
  var arg_1 = vec3<i32>(1i);
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_88ce7e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_88ce7e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_88ce7e();
}
