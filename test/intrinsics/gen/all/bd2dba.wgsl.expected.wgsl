fn all_bd2dba() {
  var res : bool = all(vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  all_bd2dba();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  all_bd2dba();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  all_bd2dba();
}
