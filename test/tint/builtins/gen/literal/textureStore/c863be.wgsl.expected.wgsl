@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg32float, write>;

fn textureStore_c863be() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_c863be();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_c863be();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c863be();
}
