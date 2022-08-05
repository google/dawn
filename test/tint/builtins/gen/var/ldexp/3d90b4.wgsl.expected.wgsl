enable f16;

fn ldexp_3d90b4() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<i32>(1);
  var res : vec2<f16> = ldexp(arg_0, arg_1);
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
