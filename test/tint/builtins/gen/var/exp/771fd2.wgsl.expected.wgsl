fn exp_771fd2() {
  var arg_0 = 1.0f;
  var res : f32 = exp(arg_0);
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
