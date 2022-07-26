fn cosh_e0c1de() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = cosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_e0c1de();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_e0c1de();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_e0c1de();
}
