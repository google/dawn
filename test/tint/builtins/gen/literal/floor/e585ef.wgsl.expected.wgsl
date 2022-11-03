fn floor_e585ef() {
  var res = floor(vec2(1.5));
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
