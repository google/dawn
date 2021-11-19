fn frexp_eabd40() {
  var res = frexp(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_eabd40();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_eabd40();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_eabd40();
}
