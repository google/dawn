fn countTrailingZeros_1dc84a() {
  var res : vec4<i32> = countTrailingZeros(vec4<i32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_1dc84a();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countTrailingZeros_1dc84a();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_1dc84a();
}
