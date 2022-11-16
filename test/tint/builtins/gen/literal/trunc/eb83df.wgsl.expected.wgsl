fn trunc_eb83df() {
  var res : f32 = trunc(1.5f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_eb83df();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_eb83df();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_eb83df();
}
