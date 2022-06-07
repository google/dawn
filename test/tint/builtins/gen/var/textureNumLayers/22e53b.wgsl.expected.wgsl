@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32sint, write>;

fn textureNumLayers_22e53b() {
  var res : i32 = textureNumLayers(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_22e53b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_22e53b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_22e53b();
}
