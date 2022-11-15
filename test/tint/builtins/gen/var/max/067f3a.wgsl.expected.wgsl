fn max_067f3a() {
  const arg_0 = vec2(1);
  const arg_1 = vec2(1);
  var res = max(arg_0, arg_1);
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
