fn ignore_6698df() {
  ignore(1u);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_6698df();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_6698df();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_6698df();
}
