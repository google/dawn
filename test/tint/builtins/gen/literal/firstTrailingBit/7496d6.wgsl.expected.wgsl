fn firstTrailingBit_7496d6() {
  var res : vec3<i32> = firstTrailingBit(vec3<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_7496d6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstTrailingBit_7496d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_7496d6();
}
