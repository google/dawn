fn ceil_b74c16() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = ceil(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_b74c16();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  ceil_b74c16();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  ceil_b74c16();
}
