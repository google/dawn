fn length_afde8b() {
  var res : f32 = length(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  length_afde8b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  length_afde8b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  length_afde8b();
}
