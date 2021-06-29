fn tanh_9f9fb9() {
  var res : vec3<f32> = tanh(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  tanh_9f9fb9();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  tanh_9f9fb9();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  tanh_9f9fb9();
}
