fn ceil_b74c16() {
  var res : vec4<f32> = ceil(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  ceil_b74c16();
}

[[stage(fragment)]]
fn fragment_main() {
  ceil_b74c16();
}

[[stage(compute)]]
fn compute_main() {
  ceil_b74c16();
}
