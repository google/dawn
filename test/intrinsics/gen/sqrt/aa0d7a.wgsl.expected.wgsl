fn sqrt_aa0d7a() {
  var res : vec4<f32> = sqrt(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sqrt_aa0d7a();
}

[[stage(fragment)]]
fn fragment_main() {
  sqrt_aa0d7a();
}

[[stage(compute)]]
fn compute_main() {
  sqrt_aa0d7a();
}
