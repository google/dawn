fn countLeadingZeros_eab32b() {
  var res : vec4<i32> = countLeadingZeros(vec4<i32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_eab32b();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countLeadingZeros_eab32b();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_eab32b();
}
