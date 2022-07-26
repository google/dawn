fn abs_9c80a6() {
  var res : vec4<i32> = abs(vec4<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_9c80a6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_9c80a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_9c80a6();
}
