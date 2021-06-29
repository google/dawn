fn exp_0f70eb() {
  var res : vec4<f32> = exp(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  exp_0f70eb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  exp_0f70eb();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  exp_0f70eb();
}
