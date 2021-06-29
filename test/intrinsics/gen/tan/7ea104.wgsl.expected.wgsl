fn tan_7ea104() {
  var res : vec3<f32> = tan(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  tan_7ea104();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  tan_7ea104();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  tan_7ea104();
}
