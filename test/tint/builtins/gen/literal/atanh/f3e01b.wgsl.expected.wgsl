fn atanh_f3e01b() {
  var res : vec4<f32> = atanh(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_f3e01b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_f3e01b();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_f3e01b();
}
