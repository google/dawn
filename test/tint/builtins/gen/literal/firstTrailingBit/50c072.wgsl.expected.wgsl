fn firstTrailingBit_50c072() {
  var res : vec2<i32> = firstTrailingBit(vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_50c072();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstTrailingBit_50c072();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_50c072();
}
