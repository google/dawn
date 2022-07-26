fn countLeadingZeros_eab32b() {
  var arg_0 = vec4<i32>(1);
  var res : vec4<i32> = countLeadingZeros(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_eab32b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countLeadingZeros_eab32b();
}

@compute @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_eab32b();
}
