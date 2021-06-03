fn sign_d065d8() {
  var res : vec2<f32> = sign(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sign_d065d8();
}

[[stage(fragment)]]
fn fragment_main() {
  sign_d065d8();
}

[[stage(compute)]]
fn compute_main() {
  sign_d065d8();
}
