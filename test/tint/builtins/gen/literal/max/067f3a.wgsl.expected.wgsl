fn max_067f3a() {
  var res = max(vec2(1), vec2(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_067f3a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_067f3a();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_067f3a();
}
