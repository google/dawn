fn atanh_e431bb() {
  var res = atanh(vec4(0.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_e431bb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_e431bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_e431bb();
}
