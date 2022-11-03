fn floor_dcd5a2() {
  const arg_0 = 1.5;
  var res = floor(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_dcd5a2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_dcd5a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_dcd5a2();
}
