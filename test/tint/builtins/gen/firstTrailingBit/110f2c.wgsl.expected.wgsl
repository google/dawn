fn firstTrailingBit_110f2c() {
  var res : vec4<u32> = firstTrailingBit(vec4<u32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_110f2c();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstTrailingBit_110f2c();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_110f2c();
}
