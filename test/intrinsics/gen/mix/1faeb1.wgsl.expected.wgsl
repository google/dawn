fn mix_1faeb1() {
  var res : vec4<f32> = mix(vec4<f32>(), vec4<f32>(), 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  mix_1faeb1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  mix_1faeb1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  mix_1faeb1();
}
