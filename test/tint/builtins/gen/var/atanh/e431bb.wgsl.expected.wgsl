fn atanh_e431bb() {
  const arg_0 = vec4(0.5);
  var res = atanh(arg_0);
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
