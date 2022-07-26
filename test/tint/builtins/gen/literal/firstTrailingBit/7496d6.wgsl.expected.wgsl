fn firstTrailingBit_7496d6() {
  var res : vec3<i32> = firstTrailingBit(vec3<i32>(1));
}

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
