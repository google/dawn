@group(1) @binding(0) var arg_0 : texture_depth_2d;

fn textureNumLevels_d63126() {
  var res : u32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_d63126();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_d63126();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_d63126();
}
