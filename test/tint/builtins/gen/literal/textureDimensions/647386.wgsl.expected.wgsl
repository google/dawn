@group(1) @binding(0) var arg_0 : texture_depth_cube;

fn textureDimensions_647386() {
  var res : vec2<i32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_647386();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_647386();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_647386();
}
