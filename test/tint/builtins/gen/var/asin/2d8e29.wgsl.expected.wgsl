enable f16;

fn asin_2d8e29() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = asin(arg_0);
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
