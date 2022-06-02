fn cos_c3b486() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = cos(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_c3b486();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  cos_c3b486();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  cos_c3b486();
}
