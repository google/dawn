enable f16;

fn round_f665b5() {
  var arg_0 = vec4<f16>(3.3984375h);
  var res : vec4<f16> = round(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_f665b5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_f665b5();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_f665b5();
}
