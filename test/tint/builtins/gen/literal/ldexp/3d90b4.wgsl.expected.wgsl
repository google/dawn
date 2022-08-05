enable f16;

fn ldexp_3d90b4() {
  var res : vec2<f16> = ldexp(vec2<f16>(f16()), vec2<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_3d90b4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_3d90b4();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_3d90b4();
}
