fn smoothStep_c11eef() {
  var res : vec2<f32> = smoothStep(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  smoothStep_c11eef();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  smoothStep_c11eef();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  smoothStep_c11eef();
}
