fn smoothStep_658be3() {
  var res : vec3<f32> = smoothStep(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  smoothStep_658be3();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  smoothStep_658be3();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  smoothStep_658be3();
}
