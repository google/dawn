fn fract_a49758() {
  var res : vec3<f32> = fract(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_a49758();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_a49758();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_a49758();
}
