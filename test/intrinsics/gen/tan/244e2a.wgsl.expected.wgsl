fn tan_244e2a() {
  var res : vec4<f32> = tan(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  tan_244e2a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  tan_244e2a();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  tan_244e2a();
}
