fn abs_002533() {
  var res : vec4<f32> = abs(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  abs_002533();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_002533();
}

[[stage(compute)]]
fn compute_main() {
  abs_002533();
}
