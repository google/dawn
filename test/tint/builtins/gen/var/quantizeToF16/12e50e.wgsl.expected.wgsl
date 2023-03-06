fn quantizeToF16_12e50e() {
  var arg_0 = 1.0f;
  var res : f32 = quantizeToF16(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

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
