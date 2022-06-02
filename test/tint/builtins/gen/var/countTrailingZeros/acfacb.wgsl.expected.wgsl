fn countTrailingZeros_acfacb() {
  var arg_0 = vec3<i32>();
  var res : vec3<i32> = countTrailingZeros(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_acfacb();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countTrailingZeros_acfacb();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_acfacb();
}
