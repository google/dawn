fn round_8fdca3() {
  var res = round(vec2(3.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_8fdca3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_8fdca3();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_8fdca3();
}
