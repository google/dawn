@group(1) @binding(0) var arg_0 : texture_3d<f32>;

fn textureDimensions_bb95d9() {
  var res : vec3<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_bb95d9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_bb95d9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_bb95d9();
}
