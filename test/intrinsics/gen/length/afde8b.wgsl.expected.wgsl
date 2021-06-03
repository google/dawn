fn length_afde8b() {
  var res : f32 = length(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  length_afde8b();
}

[[stage(fragment)]]
fn fragment_main() {
  length_afde8b();
}

[[stage(compute)]]
fn compute_main() {
  length_afde8b();
}
