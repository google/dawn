@group(1) @binding(0) var arg_0 : texture_2d<f32>;

fn textureDimensions_5331e0() {
  var res : vec2<i32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_5331e0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_5331e0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_5331e0();
}
