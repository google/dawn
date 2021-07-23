fn frexp_d80367() {
  var res = frexp(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_d80367();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_d80367();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_d80367();
}
