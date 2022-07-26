fn tanh_c15fdb() {
  var arg_0 = 1.0f;
  var res : f32 = tanh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_c15fdb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_c15fdb();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_c15fdb();
}
