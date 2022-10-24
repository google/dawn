@group(1) @binding(0) var arg_0 : texture_cube<u32>;

fn textureDimensions_8d87ed() {
  var res : vec2<i32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_8d87ed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_8d87ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_8d87ed();
}
