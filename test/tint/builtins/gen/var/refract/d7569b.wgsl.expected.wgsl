fn refract_d7569b() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  const arg_2 = 1.0;
  var res = refract(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  refract_d7569b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  refract_d7569b();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_d7569b();
}
