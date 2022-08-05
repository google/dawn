enable f16;

fn acosh_de60d8() {
  var arg_0 = vec4<f16>(f16());
  var res : vec4<f16> = acosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_de60d8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_de60d8();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_de60d8();
}
