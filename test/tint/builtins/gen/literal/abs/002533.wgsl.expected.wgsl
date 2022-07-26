fn abs_002533() {
  var res : vec4<f32> = abs(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_002533();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_002533();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_002533();
}
