fn countTrailingZeros_327c37() {
  var arg_0 = vec2<i32>();
  var res : vec2<i32> = countTrailingZeros(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_327c37();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countTrailingZeros_327c37();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_327c37();
}
