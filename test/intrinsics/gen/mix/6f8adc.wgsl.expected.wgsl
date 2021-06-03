fn mix_6f8adc() {
  var res : vec2<f32> = mix(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  mix_6f8adc();
}

[[stage(fragment)]]
fn fragment_main() {
  mix_6f8adc();
}

[[stage(compute)]]
fn compute_main() {
  mix_6f8adc();
}
