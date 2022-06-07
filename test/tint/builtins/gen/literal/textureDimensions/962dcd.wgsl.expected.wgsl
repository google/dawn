@group(1) @binding(0) var arg_0 : texture_cube<i32>;

fn textureDimensions_962dcd() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_962dcd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_962dcd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_962dcd();
}
