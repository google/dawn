fn clamp_1a32e3() {
  var arg_0 = vec4<i32>(1i);
  var arg_1 = vec4<i32>(1i);
  var arg_2 = vec4<i32>(1i);
  var res : vec4<i32> = clamp(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_1a32e3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_1a32e3();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_1a32e3();
}
