fn countLeadingZeros_70783f() {
  var arg_0 = vec2<u32>();
  var res : vec2<u32> = countLeadingZeros(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_70783f();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countLeadingZeros_70783f();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_70783f();
}
