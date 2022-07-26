fn pow_46e029() {
  var arg_0 = 1.0f;
  var arg_1 = 1.0f;
  var res : f32 = pow(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_46e029();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_46e029();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_46e029();
}
