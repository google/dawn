enable f16;

fn ldexp_624e0c() {
  var arg_0 = f16();
  var arg_1 = 1;
  var res : f16 = ldexp(arg_0, arg_1);
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
