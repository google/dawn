fn round_106c0b() {
  var res : vec4<f32> = round(vec4<f32>(3.400000095f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_106c0b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_106c0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_106c0b();
}
