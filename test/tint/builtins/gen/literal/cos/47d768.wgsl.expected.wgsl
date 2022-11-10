fn cos_47d768() {
  var res = cos(vec4(0.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_47d768();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_47d768();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_47d768();
}
