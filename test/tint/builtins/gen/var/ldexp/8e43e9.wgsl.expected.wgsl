enable f16;

fn ldexp_8e43e9() {
  var arg_0 = vec3<f16>(1.0h);
  const arg_1 = vec3(1);
  var res : vec3<f16> = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_8e43e9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_8e43e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_8e43e9();
}
