@group(1) @binding(0) var arg_0 : texture_cube_array<u32>;

fn textureDimensions_346fee() {
  var res : vec2<u32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_346fee();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_346fee();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_346fee();
}
