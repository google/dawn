fn firstTrailingBit_cb51ce() {
  var arg_0 = vec3<u32>(1u);
  var res : vec3<u32> = firstTrailingBit(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_cb51ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstTrailingBit_cb51ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_cb51ce();
}
