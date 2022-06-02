fn countOneBits_94fd81() {
  var arg_0 = vec2<u32>();
  var res : vec2<u32> = countOneBits(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_94fd81();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countOneBits_94fd81();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countOneBits_94fd81();
}
