@group(1) @binding(0) var arg_0 : texture_storage_2d<rg32sint, write>;

fn textureStore_9f5318() {
  textureStore(arg_0, vec2<u32>(1u), vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_9f5318();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_9f5318();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9f5318();
}
