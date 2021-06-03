fn mix_c37ede() {
  var res : vec4<f32> = mix(vec4<f32>(), vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  mix_c37ede();
}

[[stage(fragment)]]
fn fragment_main() {
  mix_c37ede();
}

[[stage(compute)]]
fn compute_main() {
  mix_c37ede();
}
