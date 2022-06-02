fn sqrt_f8c59a() {
  var arg_0 = vec3<f32>();
  var res : vec3<f32> = sqrt(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_f8c59a();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  sqrt_f8c59a();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  sqrt_f8c59a();
}
