fn abs_7faa9e() {
  var res : vec2<i32> = abs(vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  abs_7faa9e();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_7faa9e();
}

[[stage(compute)]]
fn compute_main() {
  abs_7faa9e();
}
