fn trunc_7d6ded() {
  const arg_0 = 1.5;
  var res = trunc(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_7d6ded();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_7d6ded();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_7d6ded();
}
