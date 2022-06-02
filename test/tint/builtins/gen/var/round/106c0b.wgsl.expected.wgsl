fn round_106c0b() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = round(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_106c0b();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  round_106c0b();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  round_106c0b();
}
