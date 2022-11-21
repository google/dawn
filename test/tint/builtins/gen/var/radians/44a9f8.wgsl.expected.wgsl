fn radians_44a9f8() {
  const arg_0 = vec2(1.0);
  var res = radians(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_44a9f8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_44a9f8();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_44a9f8();
}
