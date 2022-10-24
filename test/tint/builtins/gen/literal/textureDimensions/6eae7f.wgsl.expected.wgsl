@group(1) @binding(0) var arg_0 : texture_cube<i32>;

fn textureDimensions_6eae7f() {
  var res : vec2<i32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_6eae7f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_6eae7f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_6eae7f();
}
