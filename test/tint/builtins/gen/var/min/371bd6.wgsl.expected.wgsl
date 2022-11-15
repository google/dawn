fn min_371bd6() {
  const arg_0 = vec3(1);
  const arg_1 = vec3(1);
  var res = min(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_371bd6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_371bd6();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_371bd6();
}
