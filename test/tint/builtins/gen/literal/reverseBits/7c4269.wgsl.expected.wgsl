fn reverseBits_7c4269() {
  var res : i32 = reverseBits(1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reverseBits_7c4269();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reverseBits_7c4269();
}

@compute @workgroup_size(1)
fn compute_main() {
  reverseBits_7c4269();
}
