fn select_c31f9e() {
  var res : bool = select(true, true, true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_c31f9e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_c31f9e();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_c31f9e();
}
