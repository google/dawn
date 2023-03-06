fn min_c76fa6() {
  var arg_0 = vec4<f32>(1.0f);
  var arg_1 = vec4<f32>(1.0f);
  var res : vec4<f32> = min(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_c76fa6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_c76fa6();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_c76fa6();
}
