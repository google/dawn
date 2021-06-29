fn reflect_b61e10() {
  var res : vec2<f32> = reflect(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  reflect_b61e10();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  reflect_b61e10();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  reflect_b61e10();
}
