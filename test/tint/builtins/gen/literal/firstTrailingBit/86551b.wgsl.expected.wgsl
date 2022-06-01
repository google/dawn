fn firstTrailingBit_86551b() {
  var res : vec4<i32> = firstTrailingBit(vec4<i32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_86551b();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstTrailingBit_86551b();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_86551b();
}
