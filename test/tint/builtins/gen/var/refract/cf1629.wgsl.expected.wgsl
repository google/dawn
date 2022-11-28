fn refract_cf1629() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  const arg_2 = 1.0;
  var res = refract(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  refract_cf1629();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  refract_cf1629();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_cf1629();
}
