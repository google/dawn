enable f16;

fn sinh_69cce2() {
  var arg_0 = 1.0h;
  var res : f16 = sinh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_69cce2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_69cce2();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_69cce2();
}
