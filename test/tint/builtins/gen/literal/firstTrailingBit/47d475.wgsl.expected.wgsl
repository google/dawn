fn firstTrailingBit_47d475() {
  var res : u32 = firstTrailingBit(1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_47d475();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstTrailingBit_47d475();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_47d475();
}
