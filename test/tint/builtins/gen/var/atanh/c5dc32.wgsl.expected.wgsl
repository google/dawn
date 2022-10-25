fn atanh_c5dc32() {
  const arg_0 = 0.5;
  var res = atanh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_c5dc32();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_c5dc32();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_c5dc32();
}
