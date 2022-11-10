fn tan_7be368() {
  var res = tan(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_7be368();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_7be368();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_7be368();
}
