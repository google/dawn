fn reflect_05357e() {
  var res : vec4<f32> = reflect(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  reflect_05357e();
}

[[stage(fragment)]]
fn fragment_main() {
  reflect_05357e();
}

[[stage(compute)]]
fn compute_main() {
  reflect_05357e();
}
