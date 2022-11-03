fn floor_e585ef() {
  const arg_0 = vec2(1.5);
  var res = floor(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_e585ef();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_e585ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_e585ef();
}
