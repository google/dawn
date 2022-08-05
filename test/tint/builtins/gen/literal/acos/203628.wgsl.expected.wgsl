enable f16;

fn acos_203628() {
  var res : vec4<f16> = acos(vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_203628();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_203628();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_203628();
}
