fn acos_5e9ad2() {
  var res = acos(0.96891242171000002692);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_5e9ad2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_5e9ad2();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_5e9ad2();
}
