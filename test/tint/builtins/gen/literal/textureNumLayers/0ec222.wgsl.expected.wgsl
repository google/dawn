@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureNumLayers_0ec222() {
  var res : u32 = textureNumLayers(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_0ec222();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_0ec222();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_0ec222();
}
