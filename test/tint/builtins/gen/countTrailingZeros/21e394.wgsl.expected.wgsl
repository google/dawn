fn countTrailingZeros_21e394() {
  var res : u32 = countTrailingZeros(1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_21e394();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countTrailingZeros_21e394();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_21e394();
}
