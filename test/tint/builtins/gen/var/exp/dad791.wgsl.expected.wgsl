fn exp_dad791() {
  const arg_0 = vec4(1.0);
  var res = exp(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_dad791();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_dad791();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_dad791();
}
