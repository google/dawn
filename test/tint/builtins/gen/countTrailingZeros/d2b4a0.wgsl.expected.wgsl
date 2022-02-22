fn countTrailingZeros_d2b4a0() {
  var res : vec4<u32> = countTrailingZeros(vec4<u32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_d2b4a0();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countTrailingZeros_d2b4a0();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_d2b4a0();
}
