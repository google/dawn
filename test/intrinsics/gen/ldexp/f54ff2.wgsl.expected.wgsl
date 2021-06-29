fn ldexp_f54ff2() {
  var res : f32 = ldexp(1.0, 1u);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ldexp_f54ff2();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_f54ff2();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ldexp_f54ff2();
}
