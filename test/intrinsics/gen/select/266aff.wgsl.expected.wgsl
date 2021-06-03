fn select_266aff() {
  var res : vec2<f32> = select(vec2<f32>(), vec2<f32>(), vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_266aff();
}

[[stage(fragment)]]
fn fragment_main() {
  select_266aff();
}

[[stage(compute)]]
fn compute_main() {
  select_266aff();
}
