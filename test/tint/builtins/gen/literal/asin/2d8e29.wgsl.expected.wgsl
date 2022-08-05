enable f16;

fn asin_2d8e29() {
  var res : vec3<f16> = asin(vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_2d8e29();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_2d8e29();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_2d8e29();
}
