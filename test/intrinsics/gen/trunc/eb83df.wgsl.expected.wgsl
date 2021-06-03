fn trunc_eb83df() {
  var res : f32 = trunc(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  trunc_eb83df();
}

[[stage(fragment)]]
fn fragment_main() {
  trunc_eb83df();
}

[[stage(compute)]]
fn compute_main() {
  trunc_eb83df();
}
