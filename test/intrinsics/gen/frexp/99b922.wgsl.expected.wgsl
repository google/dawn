fn frexp_99b922() {
  var arg_1 : i32;
  var res : f32 = frexp(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_99b922();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_99b922();
}

[[stage(compute)]]
fn compute_main() {
  frexp_99b922();
}
