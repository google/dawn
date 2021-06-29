fn ldexp_db8b49() {
  var res : f32 = ldexp(1.0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ldexp_db8b49();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_db8b49();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ldexp_db8b49();
}
