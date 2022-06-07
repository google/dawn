@group(1) @binding(0) var arg_0 : texture_1d<f32>;

fn textureDimensions_002b2a() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_002b2a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_002b2a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_002b2a();
}
