enable f16;

fn modf_8dbbbf() {
  var arg_0 = -(1.5h);
  var res = modf(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_8dbbbf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_8dbbbf();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_8dbbbf();
}
