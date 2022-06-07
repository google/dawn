@group(1) @binding(0) var arg_0 : texture_storage_1d<r32sint, write>;

fn textureDimensions_0cce40() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_0cce40();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_0cce40();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_0cce40();
}
