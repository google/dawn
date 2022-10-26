@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

fn textureDimensions_0ff9a4() {
  var res : vec2<u32> = textureDimensions(arg_0, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_0ff9a4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_0ff9a4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_0ff9a4();
}
