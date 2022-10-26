@group(1) @binding(0) var arg_0 : texture_cube_array<f32>;

fn textureDimensions_b3ab5e() {
  var arg_1 = 1i;
  var res : vec2<u32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_b3ab5e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_b3ab5e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_b3ab5e();
}
