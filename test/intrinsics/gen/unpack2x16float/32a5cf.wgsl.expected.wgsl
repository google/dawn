fn unpack2x16float_32a5cf() {
  var res : vec2<f32> = unpack2x16float(1u);
}

[[stage(vertex)]]
fn vertex_main() {
  unpack2x16float_32a5cf();
}

[[stage(fragment)]]
fn fragment_main() {
  unpack2x16float_32a5cf();
}

[[stage(compute)]]
fn compute_main() {
  unpack2x16float_32a5cf();
}
