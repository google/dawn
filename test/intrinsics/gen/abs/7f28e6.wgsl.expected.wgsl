fn abs_7f28e6() {
  var res : vec2<u32> = abs(vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  abs_7f28e6();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_7f28e6();
}

[[stage(compute)]]
fn compute_main() {
  abs_7f28e6();
}
