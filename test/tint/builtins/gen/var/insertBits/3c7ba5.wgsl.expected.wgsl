fn insertBits_3c7ba5() {
  var arg_0 = vec2<u32>();
  var arg_1 = vec2<u32>();
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : vec2<u32> = insertBits(arg_0, arg_1, arg_2, arg_3);
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
