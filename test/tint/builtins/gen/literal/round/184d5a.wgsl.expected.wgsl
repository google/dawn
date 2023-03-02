fn round_184d5a() {
  var res = round(vec4(3.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_184d5a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_184d5a();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_184d5a();
}
