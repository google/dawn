fn sqrt_20c74e() {
  var res : f32 = sqrt(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sqrt_20c74e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sqrt_20c74e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sqrt_20c74e();
}
