fn countOneBits_0d0e46() {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = countOneBits(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_0d0e46();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_0d0e46();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_0d0e46();
}
