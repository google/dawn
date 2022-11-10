fn cos_a297d4() {
  var res = cos(0.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_a297d4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_a297d4();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_a297d4();
}
