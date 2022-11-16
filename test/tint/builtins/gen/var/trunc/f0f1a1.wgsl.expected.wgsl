fn trunc_f0f1a1() {
  const arg_0 = vec4(1.5);
  var res = trunc(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_f0f1a1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_f0f1a1();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_f0f1a1();
}
