fn acos_4dac75() {
  const arg_0 = vec4(0.96891242171000002692);
  var res = acos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_4dac75();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_4dac75();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_4dac75();
}
