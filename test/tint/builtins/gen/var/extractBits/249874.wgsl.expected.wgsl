fn extractBits_249874() {
  var arg_0 = 1i;
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : i32 = extractBits(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_249874();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_249874();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_249874();
}
