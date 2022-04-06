fn insertBits_e3e3a2() {
  var res : u32 = insertBits(1u, 1u, 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_e3e3a2();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  insertBits_e3e3a2();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  insertBits_e3e3a2();
}
