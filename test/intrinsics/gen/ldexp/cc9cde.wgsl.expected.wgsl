fn ldexp_cc9cde() {
  var res : vec4<f32> = ldexp(vec4<f32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  ldexp_cc9cde();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_cc9cde();
}

[[stage(compute)]]
fn compute_main() {
  ldexp_cc9cde();
}
