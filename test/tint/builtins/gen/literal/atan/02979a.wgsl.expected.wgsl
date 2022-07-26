fn atan_02979a() {
  var res : f32 = atan(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_02979a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_02979a();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_02979a();
}
