fn sqrt_f8c59a() {
  var res : vec3<f32> = sqrt(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sqrt_f8c59a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sqrt_f8c59a();
}

[[stage(compute)]]
fn compute_main() {
  sqrt_f8c59a();
}
