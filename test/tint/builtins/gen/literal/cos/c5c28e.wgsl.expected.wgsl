fn cos_c5c28e() {
  var res : f32 = cos(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_c5c28e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_c5c28e();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_c5c28e();
}
