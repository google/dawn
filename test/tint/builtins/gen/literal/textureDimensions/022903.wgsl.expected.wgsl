@group(1) @binding(0) var arg_0 : texture_1d<i32>;

fn textureDimensions_022903() {
  var res : u32 = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_022903();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_022903();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_022903();
}
