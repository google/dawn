fn dot_14bc63() {
  const arg_0 = vec2(1);
  const arg_1 = vec2(1);
  var res = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_14bc63();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_14bc63();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_14bc63();
}
