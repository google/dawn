enable f16;

fn sin_5c0712() {
  var arg_0 = vec4<f16>(1.5703125h);
  var res : vec4<f16> = sin(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_5c0712();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_5c0712();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_5c0712();
}
