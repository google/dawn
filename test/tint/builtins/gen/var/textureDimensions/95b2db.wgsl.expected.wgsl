@group(1) @binding(0) var arg_0 : texture_cube<f32>;

fn textureDimensions_95b2db() {
  var arg_1 = 1u;
  var res : vec2<i32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_95b2db();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_95b2db();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_95b2db();
}
