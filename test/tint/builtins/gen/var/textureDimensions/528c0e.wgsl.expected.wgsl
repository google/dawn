@group(1) @binding(0) var arg_0 : texture_2d_array<u32>;

fn textureDimensions_528c0e() {
  var arg_1 = 1u;
  var res : vec2<u32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_528c0e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_528c0e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_528c0e();
}
