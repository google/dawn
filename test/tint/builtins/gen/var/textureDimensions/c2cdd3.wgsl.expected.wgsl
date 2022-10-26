@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureDimensions_c2cdd3() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_c2cdd3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_c2cdd3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_c2cdd3();
}
