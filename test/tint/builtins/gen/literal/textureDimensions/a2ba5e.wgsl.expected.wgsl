@group(1) @binding(0) var arg_0 : texture_cube<i32>;

fn textureDimensions_a2ba5e() {
  var res : vec2<u32> = textureDimensions(arg_0, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_a2ba5e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_a2ba5e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_a2ba5e();
}
