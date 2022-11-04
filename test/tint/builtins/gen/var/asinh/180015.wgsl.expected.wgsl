fn asinh_180015() {
  const arg_0 = 1.0;
  var res = asinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_180015();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_180015();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_180015();
}
