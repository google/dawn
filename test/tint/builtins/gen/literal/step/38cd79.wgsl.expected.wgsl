fn step_38cd79() {
  var res = step(vec4(1.0), vec4(1.0));
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
