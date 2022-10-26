@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32float, write>;

fn textureDimensions_9cd8ad() {
  var res : vec3<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_9cd8ad();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_9cd8ad();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_9cd8ad();
}
