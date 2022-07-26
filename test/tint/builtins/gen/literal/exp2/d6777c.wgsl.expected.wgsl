fn exp2_d6777c() {
  var res : vec2<f32> = exp2(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_d6777c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_d6777c();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_d6777c();
}
