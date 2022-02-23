fn firstTrailingBit_45eb10() {
  var res : vec2<u32> = firstTrailingBit(vec2<u32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_45eb10();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstTrailingBit_45eb10();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_45eb10();
}
