fn firstTrailingBit_7496d6() {
  var arg_0 = vec3<i32>(1);
  var res : vec3<i32> = firstTrailingBit(arg_0);
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
