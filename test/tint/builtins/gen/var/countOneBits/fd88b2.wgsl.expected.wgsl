fn countOneBits_fd88b2() {
  var arg_0 = 1i;
  var res : i32 = countOneBits(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_fd88b2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_fd88b2();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_fd88b2();
}
