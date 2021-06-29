fn sign_dd790e() {
  var res : f32 = sign(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sign_dd790e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sign_dd790e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sign_dd790e();
}
