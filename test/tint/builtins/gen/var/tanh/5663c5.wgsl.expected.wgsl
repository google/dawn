fn tanh_5663c5() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = tanh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_5663c5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_5663c5();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_5663c5();
}
