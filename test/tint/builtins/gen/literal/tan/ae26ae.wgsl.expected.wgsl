fn tan_ae26ae() {
  var res = tan(vec3(1.0));
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
