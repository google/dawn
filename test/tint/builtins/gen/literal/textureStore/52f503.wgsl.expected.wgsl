@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba32sint, write>;

fn textureStore_52f503() {
  textureStore(arg_0, vec2<u32>(1u), vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_52f503();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_52f503();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_52f503();
}
