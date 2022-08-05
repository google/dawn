enable f16;

fn atan2_ca698e() {
  var res : f16 = atan2(f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_ca698e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_ca698e();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_ca698e();
}
