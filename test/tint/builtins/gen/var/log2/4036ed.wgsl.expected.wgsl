fn log2_4036ed() {
  var arg_0 = 1.0f;
  var res : f32 = log2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_4036ed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_4036ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_4036ed();
}
