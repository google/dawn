fn countLeadingZeros_f70103() {
  var arg_0 = vec4<u32>();
  var res : vec4<u32> = countLeadingZeros(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_f70103();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countLeadingZeros_f70103();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_f70103();
}
