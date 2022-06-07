@group(1) @binding(0) var arg_0 : texture_multisampled_2d<i32>;

fn textureDimensions_daf7c0() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_daf7c0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_daf7c0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_daf7c0();
}
