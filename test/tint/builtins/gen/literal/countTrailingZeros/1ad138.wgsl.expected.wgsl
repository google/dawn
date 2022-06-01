fn countTrailingZeros_1ad138() {
  var res : vec2<u32> = countTrailingZeros(vec2<u32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_1ad138();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countTrailingZeros_1ad138();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_1ad138();
}
