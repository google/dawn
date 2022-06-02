fn all_bd2dba() {
  var arg_0 = vec3<bool>();
  var res : bool = all(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  all_bd2dba();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  all_bd2dba();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  all_bd2dba();
}
