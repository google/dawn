fn step_19accd() {
  var res : vec2<f32> = step(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  step_19accd();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  step_19accd();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  step_19accd();
}
