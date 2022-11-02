fn quantizeToF16_cba294() {
  var res : vec4<f32> = quantizeToF16(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  quantizeToF16_cba294();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  quantizeToF16_cba294();
}

@compute @workgroup_size(1)
fn compute_main() {
  quantizeToF16_cba294();
}
