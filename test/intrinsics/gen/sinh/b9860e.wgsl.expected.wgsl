fn sinh_b9860e() {
  var res : vec2<f32> = sinh(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sinh_b9860e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_b9860e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sinh_b9860e();
}
