@group(1) @binding(0) var arg_0 : texture_multisampled_2d<u32>;

fn textureDimensions_e4bfd2() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_e4bfd2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_e4bfd2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_e4bfd2();
}
