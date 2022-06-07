@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8unorm, write>;

fn textureDimensions_42d4e6() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_42d4e6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_42d4e6();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_42d4e6();
}
