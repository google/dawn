fn max_b1b73a() {
  var res : vec3<u32> = max(vec3<u32>(), vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_b1b73a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_b1b73a();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_b1b73a();
}
