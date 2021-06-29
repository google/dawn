fn trunc_562d05() {
  var res : vec3<f32> = trunc(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  trunc_562d05();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  trunc_562d05();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  trunc_562d05();
}
