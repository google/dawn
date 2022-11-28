fn reflect_a8baf2() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  var res = reflect(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_a8baf2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_a8baf2();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_a8baf2();
}
