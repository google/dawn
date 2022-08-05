enable f16;

fn ldexp_624e0c() {
  var res : f16 = ldexp(f16(), 1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_624e0c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_624e0c();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_624e0c();
}
