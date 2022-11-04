fn saturate_78b37c() {
  const arg_0 = 2.0;
  var res = saturate(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_78b37c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_78b37c();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_78b37c();
}
