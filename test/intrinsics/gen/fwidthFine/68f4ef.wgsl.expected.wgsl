fn fwidthFine_68f4ef() {
  var res : vec4<f32> = fwidthFine(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidthFine_68f4ef();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthFine_68f4ef();
}

[[stage(compute)]]
fn compute_main() {
  fwidthFine_68f4ef();
}
