fn atan2_ae713e() {
  var res : vec4<f32> = atan2(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  atan2_ae713e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  atan2_ae713e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atan2_ae713e();
}
