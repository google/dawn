@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16float, write>;

fn textureDimensions_1e9e39() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_1e9e39();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_1e9e39();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_1e9e39();
}
