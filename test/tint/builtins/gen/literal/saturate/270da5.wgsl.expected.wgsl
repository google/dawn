fn saturate_270da5() {
  var res : f32 = saturate(1.0f);
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
