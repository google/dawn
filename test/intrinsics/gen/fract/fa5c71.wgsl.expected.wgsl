fn fract_fa5c71() {
  var res : f32 = fract(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fract_fa5c71();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fract_fa5c71();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  fract_fa5c71();
}
