enable f16;

fn sinh_924f19() {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = sinh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_924f19();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_924f19();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_924f19();
}
