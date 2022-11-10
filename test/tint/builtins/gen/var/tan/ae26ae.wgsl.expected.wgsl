fn tan_ae26ae() {
  const arg_0 = vec3(1.0);
  var res = tan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_ae26ae();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_ae26ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_ae26ae();
}
