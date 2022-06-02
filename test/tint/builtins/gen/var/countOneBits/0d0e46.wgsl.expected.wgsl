fn countOneBits_0d0e46() {
  var arg_0 = vec4<u32>();
  var res : vec4<u32> = countOneBits(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_0d0e46();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countOneBits_0d0e46();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countOneBits_0d0e46();
}
