enable f16;

fn round_f665b5() {
  var res : vec4<f16> = round(vec4<f16>(1.0h));
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
