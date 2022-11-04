fn step_f9b70c() {
  var res = step(1.0, 1.0);
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
