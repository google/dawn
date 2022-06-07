@group(1) @binding(0) var arg_0 : texture_storage_1d<r32float, write>;

fn textureDimensions_a863f2() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_a863f2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_a863f2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_a863f2();
}
