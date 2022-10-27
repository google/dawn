fn step_f9b70c() {
  const arg_0 = 1;
  const arg_1 = 1;
  var res = step(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_f9b70c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_f9b70c();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_f9b70c();
}
