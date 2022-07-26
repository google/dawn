fn cosh_e0c1de() {
  var res : vec4<f32> = cosh(vec4<f32>(1.0f));
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
