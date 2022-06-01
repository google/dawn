fn firstTrailingBit_cb51ce() {
  var res : vec3<u32> = firstTrailingBit(vec3<u32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_cb51ce();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstTrailingBit_cb51ce();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_cb51ce();
}
