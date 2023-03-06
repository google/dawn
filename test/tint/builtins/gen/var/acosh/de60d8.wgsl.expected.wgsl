enable f16;

fn acosh_de60d8() {
  var arg_0 = vec4<f16>(1.54296875h);
  var res : vec4<f16> = acosh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_de60d8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_de60d8();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_de60d8();
}
