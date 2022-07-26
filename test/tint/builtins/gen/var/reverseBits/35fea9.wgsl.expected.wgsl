fn reverseBits_35fea9() {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = reverseBits(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reverseBits_35fea9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reverseBits_35fea9();
}

@compute @workgroup_size(1)
fn compute_main() {
  reverseBits_35fea9();
}
