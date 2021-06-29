fn cosh_377652() {
  var res : vec3<f32> = cosh(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  cosh_377652();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  cosh_377652();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  cosh_377652();
}
