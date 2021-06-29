fn pow_04a908() {
  var res : vec4<f32> = pow(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  pow_04a908();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  pow_04a908();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  pow_04a908();
}
