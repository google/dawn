@group(1) @binding(0) var arg_0 : texture_cube<f32>;

fn textureDimensions_49a067() {
  var res : vec2<u32> = textureDimensions(arg_0, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_49a067();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_49a067();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_49a067();
}
