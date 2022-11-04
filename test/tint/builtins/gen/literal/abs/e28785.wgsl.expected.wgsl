fn abs_e28785() {
  var res = abs(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_e28785();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_e28785();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_e28785();
}
