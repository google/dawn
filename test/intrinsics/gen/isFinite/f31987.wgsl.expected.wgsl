fn isFinite_f31987() {
  var res : vec4<bool> = isFinite(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  isFinite_f31987();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_f31987();
}

[[stage(compute)]]
fn compute_main() {
  isFinite_f31987();
}
