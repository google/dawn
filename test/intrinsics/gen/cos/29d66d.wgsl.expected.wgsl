fn cos_29d66d() {
  var res : vec4<f32> = cos(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  cos_29d66d();
}

[[stage(fragment)]]
fn fragment_main() {
  cos_29d66d();
}

[[stage(compute)]]
fn compute_main() {
  cos_29d66d();
}
