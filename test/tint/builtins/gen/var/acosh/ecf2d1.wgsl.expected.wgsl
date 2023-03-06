fn acosh_ecf2d1() {
  var arg_0 = 1.54308068752288818359f;
  var res : f32 = acosh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_ecf2d1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_ecf2d1();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_ecf2d1();
}
