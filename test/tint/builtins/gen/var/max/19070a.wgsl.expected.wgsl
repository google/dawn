fn max_19070a() {
  const arg_0 = vec4(1);
  const arg_1 = vec4(1);
  var res = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_19070a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_19070a();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_19070a();
}
