fn asin_8cd9c9() {
  var arg_0 = vec3<f32>();
  var res : vec3<f32> = asin(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_8cd9c9();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  asin_8cd9c9();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  asin_8cd9c9();
}
