@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32float, write>;

fn textureDimensions_55b23e() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_55b23e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_55b23e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_55b23e();
}
