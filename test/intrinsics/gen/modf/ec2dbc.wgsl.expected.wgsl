fn modf_ec2dbc() {
  var res = modf(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_ec2dbc();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_ec2dbc();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_ec2dbc();
}
