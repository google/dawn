enable f16;

fn acos_004aff() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = acos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_004aff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_004aff();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_004aff();
}
