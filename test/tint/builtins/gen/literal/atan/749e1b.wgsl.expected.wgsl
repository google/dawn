fn atan_749e1b() {
  var res = atan(vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_749e1b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_749e1b();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_749e1b();
}
