enable f16;

fn acosh_a37dfe() {
  var arg_0 = f16();
  var res : f16 = acosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_a37dfe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_a37dfe();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_a37dfe();
}
