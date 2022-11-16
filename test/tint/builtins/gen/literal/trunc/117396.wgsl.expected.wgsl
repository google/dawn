fn trunc_117396() {
  var res = trunc(vec3(1.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_117396();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_117396();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_117396();
}
