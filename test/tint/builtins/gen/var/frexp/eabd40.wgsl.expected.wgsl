fn frexp_eabd40() {
  var arg_0 = 1.0f;
  var res = frexp(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_eabd40();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_eabd40();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_eabd40();
}
