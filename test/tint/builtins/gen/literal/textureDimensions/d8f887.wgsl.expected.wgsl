@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8uint, write>;

fn textureDimensions_d8f887() {
  var res : vec3<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_d8f887();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_d8f887();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_d8f887();
}
