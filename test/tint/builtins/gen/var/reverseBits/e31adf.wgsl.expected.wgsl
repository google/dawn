fn reverseBits_e31adf() {
  var arg_0 = 1u;
  var res : u32 = reverseBits(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reverseBits_e31adf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reverseBits_e31adf();
}

@compute @workgroup_size(1)
fn compute_main() {
  reverseBits_e31adf();
}
