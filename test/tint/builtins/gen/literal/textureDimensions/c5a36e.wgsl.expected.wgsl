@group(1) @binding(0) var arg_0 : texture_depth_cube;

fn textureDimensions_c5a36e() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_c5a36e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_c5a36e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_c5a36e();
}
