fn step_38cd79() {
  const arg_0 = vec4(1.0);
  const arg_1 = vec4(1.0);
  var res = step(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_38cd79();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_38cd79();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_38cd79();
}
