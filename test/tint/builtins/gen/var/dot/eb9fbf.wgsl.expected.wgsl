fn dot_eb9fbf() {
  const arg_0 = vec4(1);
  const arg_1 = vec4(1);
  var res = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_eb9fbf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_eb9fbf();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_eb9fbf();
}
