fn cos_af7447() {
  var res = cos(vec2(0.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_af7447();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_af7447();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_af7447();
}
