fn countLeadingZeros_7c38a6() {
  var arg_0 = vec3<i32>();
  var res : vec3<i32> = countLeadingZeros(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_7c38a6();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countLeadingZeros_7c38a6();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_7c38a6();
}
