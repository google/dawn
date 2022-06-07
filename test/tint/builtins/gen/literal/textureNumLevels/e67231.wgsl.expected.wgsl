@group(1) @binding(0) var arg_0 : texture_2d<f32>;

fn textureNumLevels_e67231() {
  var res : i32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_e67231();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_e67231();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_e67231();
}
