fn modf_c15f48() {
  const arg_0 = -(1.5);
  var res = modf(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_c15f48();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_c15f48();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_c15f48();
}
