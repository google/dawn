enable f16;

fn modf_a545b9() {
  var arg_0 = vec2<f16>(-(1.5h));
  var res = modf(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_a545b9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_a545b9();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_a545b9();
}
