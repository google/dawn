enable f16;

fn ldexp_217a31() {
  var arg_0 = vec2<f16>(1.0h);
  const arg_1 = vec2(1);
  var res : vec2<f16> = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_217a31();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_217a31();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_217a31();
}
