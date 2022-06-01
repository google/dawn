fn insertBits_65468b() {
  var res : i32 = insertBits(1, 1, 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_65468b();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  insertBits_65468b();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  insertBits_65468b();
}
