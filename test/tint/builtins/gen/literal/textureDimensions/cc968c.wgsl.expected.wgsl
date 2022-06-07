@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32sint, write>;

fn textureDimensions_cc968c() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_cc968c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_cc968c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_cc968c();
}
