@group(1) @binding(0) var arg_0 : texture_1d<f32>;

fn textureDimensions_26d6bf() {
  var res : u32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_26d6bf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_26d6bf();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_26d6bf();
}
