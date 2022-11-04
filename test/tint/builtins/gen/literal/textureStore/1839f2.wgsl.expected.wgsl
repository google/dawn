@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8sint, write>;

fn textureStore_1839f2() {
  textureStore(arg_0, vec2<u32>(1u), vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_1839f2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_1839f2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_1839f2();
}
