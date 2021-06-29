fn fma_e17c5c() {
  var res : vec3<f32> = fma(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fma_e17c5c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fma_e17c5c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  fma_e17c5c();
}
