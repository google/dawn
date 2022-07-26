fn round_9edc38() {
  var res : f32 = round(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_9edc38();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_9edc38();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_9edc38();
}
