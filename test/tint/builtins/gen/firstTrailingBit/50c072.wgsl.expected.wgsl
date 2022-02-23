fn firstTrailingBit_50c072() {
  var res : vec2<i32> = firstTrailingBit(vec2<i32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_50c072();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstTrailingBit_50c072();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_50c072();
}
