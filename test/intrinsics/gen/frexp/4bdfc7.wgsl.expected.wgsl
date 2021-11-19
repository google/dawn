fn frexp_4bdfc7() {
  var res = frexp(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_4bdfc7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_4bdfc7();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_4bdfc7();
}
