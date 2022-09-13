fn saturate_270da5() {
  var arg_0 = 1.0f;
  var res : f32 = saturate(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_270da5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_270da5();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_270da5();
}
