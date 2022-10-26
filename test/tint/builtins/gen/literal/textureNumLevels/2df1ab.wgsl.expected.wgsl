@group(1) @binding(0) var arg_0 : texture_cube<f32>;

fn textureNumLevels_2df1ab() {
  var res : u32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_2df1ab();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_2df1ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_2df1ab();
}
