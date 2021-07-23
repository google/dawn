fn frexp_12f1da() {
  var res = frexp(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_12f1da();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_12f1da();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_12f1da();
}
