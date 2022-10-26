@group(1) @binding(0) var arg_0 : texture_1d<f32>;

fn textureDimensions_f17acd() {
  var arg_1 = 1i;
  var res : u32 = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_f17acd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_f17acd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_f17acd();
}
