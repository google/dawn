fn ldexp_f54ff2() {
  var res : f32 = ldexp(1.0, 1u);
}

[[stage(vertex)]]
fn vertex_main() {
  ldexp_f54ff2();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_f54ff2();
}

[[stage(compute)]]
fn compute_main() {
  ldexp_f54ff2();
}
