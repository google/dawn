fn sqrt_8c7024() {
  var res : vec2<f32> = sqrt(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sqrt_8c7024();
}

[[stage(fragment)]]
fn fragment_main() {
  sqrt_8c7024();
}

[[stage(compute)]]
fn compute_main() {
  sqrt_8c7024();
}
