fn dot_0c577b() {
  var res : f32 = dot(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  dot_0c577b();
}

[[stage(fragment)]]
fn fragment_main() {
  dot_0c577b();
}

[[stage(compute)]]
fn compute_main() {
  dot_0c577b();
}
