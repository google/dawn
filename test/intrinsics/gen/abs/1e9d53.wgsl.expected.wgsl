fn abs_1e9d53() {
  var res : vec2<f32> = abs(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  abs_1e9d53();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_1e9d53();
}

[[stage(compute)]]
fn compute_main() {
  abs_1e9d53();
}
