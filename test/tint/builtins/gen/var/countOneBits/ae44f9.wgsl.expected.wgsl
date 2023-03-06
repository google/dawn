fn countOneBits_ae44f9() {
  var arg_0 = 1u;
  var res : u32 = countOneBits(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_ae44f9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_ae44f9();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_ae44f9();
}
