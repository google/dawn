fn round_106c0b() {
  var arg_0 = vec4<f32>(3.400000095f);
  var res : vec4<f32> = round(arg_0);
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
