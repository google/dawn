@group(1) @binding(0) var arg_0 : texture_2d<f32>;

fn textureDimensions_2fe1cc() {
  var res : vec2<i32> = textureDimensions(arg_0, 0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_2fe1cc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_2fe1cc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_2fe1cc();
}
