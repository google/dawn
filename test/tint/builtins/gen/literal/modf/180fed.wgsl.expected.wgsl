fn modf_180fed() {
  var res = modf(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_180fed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_180fed();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_180fed();
}
