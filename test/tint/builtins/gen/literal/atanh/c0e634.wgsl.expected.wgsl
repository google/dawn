fn atanh_c0e634() {
  var res : vec2<f32> = atanh(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_c0e634();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_c0e634();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_c0e634();
}
