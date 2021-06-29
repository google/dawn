fn pow_46e029() {
  var res : f32 = pow(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  pow_46e029();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  pow_46e029();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  pow_46e029();
}
