fn abs_9c80a6() {
  var res : vec4<i32> = abs(vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  abs_9c80a6();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_9c80a6();
}

[[stage(compute)]]
fn compute_main() {
  abs_9c80a6();
}
