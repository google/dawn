fn asinh_8d2e51() {
  var res : vec4<f32> = asinh(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_8d2e51();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_8d2e51();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_8d2e51();
}
