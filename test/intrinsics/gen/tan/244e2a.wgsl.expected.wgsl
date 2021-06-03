fn tan_244e2a() {
  var res : vec4<f32> = tan(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  tan_244e2a();
}

[[stage(fragment)]]
fn fragment_main() {
  tan_244e2a();
}

[[stage(compute)]]
fn compute_main() {
  tan_244e2a();
}
