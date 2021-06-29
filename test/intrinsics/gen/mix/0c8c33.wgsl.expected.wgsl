fn mix_0c8c33() {
  var res : vec3<f32> = mix(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  mix_0c8c33();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  mix_0c8c33();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  mix_0c8c33();
}
