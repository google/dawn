fn countTrailingZeros_8ed26f() {
  var res : vec3<u32> = countTrailingZeros(vec3<u32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_8ed26f();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countTrailingZeros_8ed26f();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_8ed26f();
}
