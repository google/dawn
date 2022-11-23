fn modf_732aa6() {
  var res = modf(vec2(-(1.5)));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_732aa6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_732aa6();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_732aa6();
}
