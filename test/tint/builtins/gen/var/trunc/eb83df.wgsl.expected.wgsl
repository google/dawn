fn trunc_eb83df() {
  var arg_0 = 1.5f;
  var res : f32 = trunc(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

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
