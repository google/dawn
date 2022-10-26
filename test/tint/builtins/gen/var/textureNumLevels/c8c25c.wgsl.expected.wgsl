@group(1) @binding(0) var arg_0 : texture_depth_cube;

fn textureNumLevels_c8c25c() {
  var res : u32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_c8c25c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_c8c25c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_c8c25c();
}
