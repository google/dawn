@group(1) @binding(0) var arg_0 : texture_2d<u32>;

fn textureDimensions_220dc3() {
  var res : vec2<i32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_220dc3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_220dc3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_220dc3();
}
