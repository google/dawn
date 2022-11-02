fn quantizeToF16_2cddf3() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = quantizeToF16(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  quantizeToF16_2cddf3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  quantizeToF16_2cddf3();
}

@compute @workgroup_size(1)
fn compute_main() {
  quantizeToF16_2cddf3();
}
