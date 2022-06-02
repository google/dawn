fn insertBits_428b0b() {
  var arg_0 = vec3<i32>();
  var arg_1 = vec3<i32>();
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : vec3<i32> = insertBits(arg_0, arg_1, arg_2, arg_3);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_428b0b();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  insertBits_428b0b();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  insertBits_428b0b();
}
