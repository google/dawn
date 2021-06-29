fn exp_1951e7() {
  var res : vec2<f32> = exp(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  exp_1951e7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  exp_1951e7();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  exp_1951e7();
}
