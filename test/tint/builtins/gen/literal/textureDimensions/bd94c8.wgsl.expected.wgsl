@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

fn textureDimensions_bd94c8() {
  var res : vec2<u32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_bd94c8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_bd94c8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_bd94c8();
}
