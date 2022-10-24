@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba16sint, write>;

fn textureStore_b76ff3() {
  textureStore(arg_0, vec2<u32>(), vec4<i32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_b76ff3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_b76ff3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_b76ff3();
}
