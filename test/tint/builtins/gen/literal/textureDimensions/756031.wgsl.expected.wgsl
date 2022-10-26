@group(1) @binding(0) var arg_0 : texture_3d<i32>;

fn textureDimensions_756031() {
  var res : vec3<u32> = textureDimensions(arg_0, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_756031();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_756031();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_756031();
}
