@group(1) @binding(0) var arg_0 : texture_1d<i32>;

fn textureNumLevels_32a0ae() {
  var res : i32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_32a0ae();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_32a0ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_32a0ae();
}
