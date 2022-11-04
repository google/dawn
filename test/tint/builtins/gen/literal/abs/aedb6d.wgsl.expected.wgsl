fn abs_aedb6d() {
  var res = abs(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_aedb6d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_aedb6d();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_aedb6d();
}
