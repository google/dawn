fn refract_cf1629() {
  var res = refract(vec2(1.0), vec2(1.0), 1.0);
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
