fn firstTrailingBit_45eb10() {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = firstTrailingBit(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_45eb10();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstTrailingBit_45eb10();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_45eb10();
}
