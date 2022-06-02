fn countOneBits_ae44f9() {
  var arg_0 = 1u;
  var res : u32 = countOneBits(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_ae44f9();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countOneBits_ae44f9();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countOneBits_ae44f9();
}
