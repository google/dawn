fn modf_c87851() {
  var res = modf(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_c87851();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_c87851();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_c87851();
}
