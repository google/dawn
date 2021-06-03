fn countOneBits_af90e2() {
  var res : vec2<i32> = countOneBits(vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_af90e2();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_af90e2();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_af90e2();
}
