@group(1) @binding(0) var arg_0 : texture_depth_cube;

fn textureDimensions_d3accd() {
  var arg_1 = 1u;
  var res : vec2<u32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_d3accd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_d3accd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_d3accd();
}
