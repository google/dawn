fn countOneBits_af90e2() {
  var res : vec2<i32> = countOneBits(vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  countOneBits_af90e2();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_af90e2();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  countOneBits_af90e2();
}
