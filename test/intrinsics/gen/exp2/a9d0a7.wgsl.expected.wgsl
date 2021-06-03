fn exp2_a9d0a7() {
  var res : vec4<f32> = exp2(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  exp2_a9d0a7();
}

[[stage(fragment)]]
fn fragment_main() {
  exp2_a9d0a7();
}

[[stage(compute)]]
fn compute_main() {
  exp2_a9d0a7();
}
