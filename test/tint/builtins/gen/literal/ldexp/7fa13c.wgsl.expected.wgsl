enable f16;

fn ldexp_7fa13c() {
  var res : vec4<f16> = ldexp(vec4<f16>(f16()), vec4<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_7fa13c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_7fa13c();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_7fa13c();
}
