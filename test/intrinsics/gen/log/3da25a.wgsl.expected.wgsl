fn log_3da25a() {
  var res : vec4<f32> = log(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  log_3da25a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  log_3da25a();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  log_3da25a();
}
