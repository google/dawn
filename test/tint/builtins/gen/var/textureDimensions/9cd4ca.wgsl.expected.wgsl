@group(1) @binding(0) var arg_0 : texture_cube<u32>;

fn textureDimensions_9cd4ca() {
  var arg_1 = 1i;
  var res : vec2<u32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_9cd4ca();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_9cd4ca();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_9cd4ca();
}
