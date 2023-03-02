fn acos_069188() {
  const arg_0 = vec3(0.96891242171000002692);
  var res = acos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_069188();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_069188();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_069188();
}
