fn atan_ad96e4() {
  var res : vec2<f32> = atan(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_ad96e4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_ad96e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_ad96e4();
}
