fn sign_b8f634() {
  var res : vec4<f32> = sign(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sign_b8f634();
}

[[stage(fragment)]]
fn fragment_main() {
  sign_b8f634();
}

[[stage(compute)]]
fn compute_main() {
  sign_b8f634();
}
