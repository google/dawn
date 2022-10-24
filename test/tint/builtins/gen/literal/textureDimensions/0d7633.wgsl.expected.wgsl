@group(1) @binding(0) var arg_0 : texture_1d<i32>;

fn textureDimensions_0d7633() {
  var res : i32 = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_0d7633();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_0d7633();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_0d7633();
}
