fn radians_379214() {
  const arg_0 = vec3(1.0);
  var res = radians(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_379214();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_379214();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_379214();
}
