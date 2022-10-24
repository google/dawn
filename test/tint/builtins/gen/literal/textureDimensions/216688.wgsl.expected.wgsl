@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureDimensions_216688() {
  var res : vec2<i32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_216688();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_216688();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_216688();
}
