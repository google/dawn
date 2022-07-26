fn refract_cbc1d2() {
  var res : vec3<f32> = refract(vec3<f32>(1.0f), vec3<f32>(1.0f), 1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  refract_cbc1d2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  refract_cbc1d2();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_cbc1d2();
}
