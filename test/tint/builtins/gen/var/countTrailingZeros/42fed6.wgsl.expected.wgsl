fn countTrailingZeros_42fed6() {
  var arg_0 = 1i;
  var res : i32 = countTrailingZeros(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_42fed6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countTrailingZeros_42fed6();
}

@compute @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_42fed6();
}
