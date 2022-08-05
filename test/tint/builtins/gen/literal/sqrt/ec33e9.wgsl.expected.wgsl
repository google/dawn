enable f16;

fn sqrt_ec33e9() {
  var res : f16 = sqrt(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_ec33e9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_ec33e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_ec33e9();
}
