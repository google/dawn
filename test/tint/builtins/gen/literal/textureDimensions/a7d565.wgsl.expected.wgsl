@group(1) @binding(0) var arg_0 : texture_1d<u32>;

fn textureDimensions_a7d565() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_a7d565();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_a7d565();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_a7d565();
}
