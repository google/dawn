fn trunc_e183aa() {
  var res : vec4<f32> = trunc(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  trunc_e183aa();
}

[[stage(fragment)]]
fn fragment_main() {
  trunc_e183aa();
}

[[stage(compute)]]
fn compute_main() {
  trunc_e183aa();
}
