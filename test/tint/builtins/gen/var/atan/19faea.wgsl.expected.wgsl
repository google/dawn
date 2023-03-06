enable f16;

fn atan_19faea() {
  var arg_0 = vec4<f16>(1.0h);
  var res : vec4<f16> = atan(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_19faea();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_19faea();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_19faea();
}
