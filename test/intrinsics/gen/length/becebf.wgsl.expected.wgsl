fn length_becebf() {
  var res : f32 = length(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  length_becebf();
}

[[stage(fragment)]]
fn fragment_main() {
  length_becebf();
}

[[stage(compute)]]
fn compute_main() {
  length_becebf();
}
