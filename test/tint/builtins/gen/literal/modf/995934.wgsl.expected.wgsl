enable f16;

fn modf_995934() {
  var res = modf(vec4<f16>(-(1.5h)));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_995934();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_995934();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_995934();
}
