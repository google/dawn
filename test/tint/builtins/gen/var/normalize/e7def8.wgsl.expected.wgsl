fn normalize_e7def8() {
  const arg_0 = vec3(1.0);
  var res = normalize(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_e7def8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_e7def8();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_e7def8();
}
