fn tan_7ea104() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = tan(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_7ea104();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_7ea104();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_7ea104();
}
