fn countTrailingZeros_42fed6() {
  var res : i32 = countTrailingZeros(1i);
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
