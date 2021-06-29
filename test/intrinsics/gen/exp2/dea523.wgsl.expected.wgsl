fn exp2_dea523() {
  var res : f32 = exp2(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  exp2_dea523();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  exp2_dea523();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  exp2_dea523();
}
