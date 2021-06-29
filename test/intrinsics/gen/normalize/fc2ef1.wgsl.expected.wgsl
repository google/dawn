fn normalize_fc2ef1() {
  var res : vec2<f32> = normalize(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  normalize_fc2ef1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  normalize_fc2ef1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  normalize_fc2ef1();
}
