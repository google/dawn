fn exp2_d6777c() {
  var res : vec2<f32> = exp2(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  exp2_d6777c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  exp2_d6777c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  exp2_d6777c();
}
