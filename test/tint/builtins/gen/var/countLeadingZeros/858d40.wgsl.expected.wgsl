fn countLeadingZeros_858d40() {
  var arg_0 = vec2<i32>();
  var res : vec2<i32> = countLeadingZeros(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_858d40();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countLeadingZeros_858d40();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_858d40();
}
