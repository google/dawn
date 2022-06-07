@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32uint, write>;

fn textureDimensions_820596() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_820596();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_820596();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_820596();
}
