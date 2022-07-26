fn acos_489247() {
  var res : f32 = acos(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_489247();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_489247();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_489247();
}
