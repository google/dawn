fn exp_771fd2() {
  var res : f32 = exp(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_771fd2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_771fd2();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_771fd2();
}
