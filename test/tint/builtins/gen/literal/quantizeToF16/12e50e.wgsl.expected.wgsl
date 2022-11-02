fn quantizeToF16_12e50e() {
  var res : f32 = quantizeToF16(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  quantizeToF16_12e50e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  quantizeToF16_12e50e();
}

@compute @workgroup_size(1)
fn compute_main() {
  quantizeToF16_12e50e();
}
