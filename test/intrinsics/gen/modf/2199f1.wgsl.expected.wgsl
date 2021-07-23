fn modf_2199f1() {
  var res = modf(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_2199f1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_2199f1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_2199f1();
}
