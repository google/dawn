@group(1) @binding(0) var arg_0 : texture_2d<i32>;

fn textureDimensions_b0e16d() {
  var arg_1 = 0;
  var res : vec2<i32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_b0e16d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_b0e16d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_b0e16d();
}
