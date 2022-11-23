fn dot_c11efe() {
  const arg_0 = vec3(1);
  const arg_1 = vec3(1);
  var res = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_c11efe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_c11efe();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_c11efe();
}
