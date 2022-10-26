@group(1) @binding(0) var arg_0 : texture_cube<i32>;

fn textureDimensions_f626b3() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_f626b3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_f626b3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_f626b3();
}
