fn round_a1673d() {
  var res = round(vec3(3.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_a1673d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_a1673d();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_a1673d();
}
