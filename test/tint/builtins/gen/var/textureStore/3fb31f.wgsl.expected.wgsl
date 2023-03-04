@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32uint, write>;

fn textureStore_3fb31f() {
  var arg_1 = vec3<u32>(1u);
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_3fb31f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_3fb31f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_3fb31f();
}
