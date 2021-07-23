fn mix_315264() {
  var res : vec3<f32> = mix(vec3<f32>(), vec3<f32>(), 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  mix_315264();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  mix_315264();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  mix_315264();
}
