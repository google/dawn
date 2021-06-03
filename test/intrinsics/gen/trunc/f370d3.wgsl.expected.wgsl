fn trunc_f370d3() {
  var res : vec2<f32> = trunc(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  trunc_f370d3();
}

[[stage(fragment)]]
fn fragment_main() {
  trunc_f370d3();
}

[[stage(compute)]]
fn compute_main() {
  trunc_f370d3();
}
