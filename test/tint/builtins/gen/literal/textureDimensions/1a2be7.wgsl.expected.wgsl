@group(1) @binding(0) var arg_0 : texture_3d<i32>;

fn textureDimensions_1a2be7() {
  var res : vec3<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_1a2be7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_1a2be7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_1a2be7();
}
