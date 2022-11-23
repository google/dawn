fn exp_49e4c5() {
  const arg_0 = 1.0;
  var res = exp(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_49e4c5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_49e4c5();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_49e4c5();
}
