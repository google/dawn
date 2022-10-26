@group(1) @binding(0) var arg_0 : texture_cube_array<u32>;

fn textureDimensions_22b5b6() {
  var res : vec2<u32> = textureDimensions(arg_0, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_22b5b6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_22b5b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_22b5b6();
}
