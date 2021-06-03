fn clamp_1a32e3() {
  var res : vec4<i32> = clamp(vec4<i32>(), vec4<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_1a32e3();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_1a32e3();
}

[[stage(compute)]]
fn compute_main() {
  clamp_1a32e3();
}
