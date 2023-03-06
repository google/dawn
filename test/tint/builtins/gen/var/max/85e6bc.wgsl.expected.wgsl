fn max_85e6bc() {
  var arg_0 = vec4<i32>(1i);
  var arg_1 = vec4<i32>(1i);
  var res : vec4<i32> = max(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_85e6bc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_85e6bc();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_85e6bc();
}
