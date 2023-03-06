fn acosh_d51ccb() {
  var arg_0 = vec4<f32>(1.54308068752288818359f);
  var res : vec4<f32> = acosh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_d51ccb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_d51ccb();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_d51ccb();
}
