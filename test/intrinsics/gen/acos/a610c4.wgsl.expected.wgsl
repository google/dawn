fn acos_a610c4() {
  var res : vec3<f32> = acos(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  acos_a610c4();
}

[[stage(fragment)]]
fn fragment_main() {
  acos_a610c4();
}

[[stage(compute)]]
fn compute_main() {
  acos_a610c4();
}
