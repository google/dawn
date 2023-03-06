fn exp_0f70eb() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = exp(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_0f70eb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_0f70eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_0f70eb();
}
