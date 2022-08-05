enable f16;

fn acosh_a37dfe() {
  var res : f16 = acosh(f16());
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
