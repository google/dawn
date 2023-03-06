fn abs_4ad288() {
  var arg_0 = 1i;
  var res : i32 = abs(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_4ad288();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_4ad288();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_4ad288();
}
