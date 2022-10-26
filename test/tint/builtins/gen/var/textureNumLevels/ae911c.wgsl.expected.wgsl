@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

fn textureNumLevels_ae911c() {
  var res : u32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_ae911c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_ae911c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_ae911c();
}
