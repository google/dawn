fn mix_6f8adc() {
  var res : vec2<f32> = mix(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  mix_6f8adc();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  mix_6f8adc();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  mix_6f8adc();
}
