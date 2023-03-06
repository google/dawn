fn acosh_e38f5c() {
  var res : vec3<f32> = acosh(vec3<f32>(1.54308068752288818359f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_e38f5c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_e38f5c();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_e38f5c();
}
