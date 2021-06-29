fn ldexp_cc9cde() {
  var res : vec4<f32> = ldexp(vec4<f32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ldexp_cc9cde();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_cc9cde();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ldexp_cc9cde();
}
