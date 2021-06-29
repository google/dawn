fn pow_e60ea5() {
  var res : vec2<f32> = pow(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  pow_e60ea5();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  pow_e60ea5();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  pow_e60ea5();
}
