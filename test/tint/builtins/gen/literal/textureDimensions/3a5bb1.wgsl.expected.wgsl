@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba32float, write>;

fn textureDimensions_3a5bb1() {
  var res : vec3<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_3a5bb1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_3a5bb1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_3a5bb1();
}
