fn sqrt_4ac2c5() {
  var res = sqrt(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_4ac2c5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_4ac2c5();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_4ac2c5();
}
