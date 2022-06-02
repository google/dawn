fn insertBits_e3e3a2() {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : u32 = insertBits(arg_0, arg_1, arg_2, arg_3);
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
