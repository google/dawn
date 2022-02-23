fn insertBits_3c7ba5() {
  var res : vec2<u32> = insertBits(vec2<u32>(), vec2<u32>(), 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_3c7ba5();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  insertBits_3c7ba5();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  insertBits_3c7ba5();
}
