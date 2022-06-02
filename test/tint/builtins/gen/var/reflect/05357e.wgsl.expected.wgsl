fn reflect_05357e() {
  var arg_0 = vec4<f32>();
  var arg_1 = vec4<f32>();
  var res : vec4<f32> = reflect(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_05357e();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  reflect_05357e();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  reflect_05357e();
}
