fn round_52c84d() {
  var res : vec2<f32> = round(vec2<f32>(3.400000095f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_52c84d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_52c84d();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_52c84d();
}
