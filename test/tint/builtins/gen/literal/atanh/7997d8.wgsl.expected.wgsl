fn atanh_7997d8() {
  var res : f32 = atanh(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_7997d8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_7997d8();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_7997d8();
}
