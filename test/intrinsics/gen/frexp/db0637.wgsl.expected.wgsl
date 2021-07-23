fn frexp_db0637() {
  var res = frexp(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_db0637();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_db0637();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_db0637();
}
