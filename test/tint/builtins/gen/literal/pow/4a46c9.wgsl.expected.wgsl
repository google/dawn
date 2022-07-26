fn pow_4a46c9() {
  var res : vec3<f32> = pow(vec3<f32>(1.0f), vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_4a46c9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_4a46c9();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_4a46c9();
}
