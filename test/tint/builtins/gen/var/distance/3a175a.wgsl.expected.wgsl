fn distance_3a175a() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  var res = distance(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_3a175a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_3a175a();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_3a175a();
}
