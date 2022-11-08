fn cross_1d7933() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  var res = cross(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cross_1d7933();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cross_1d7933();
}

@compute @workgroup_size(1)
fn compute_main() {
  cross_1d7933();
}
