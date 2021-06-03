fn fwidth_d2ab9a() {
  var res : vec4<f32> = fwidth(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fwidth_d2ab9a();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_d2ab9a();
}

[[stage(compute)]]
fn compute_main() {
  fwidth_d2ab9a();
}
