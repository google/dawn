fn round_9edc38() {
  var arg_0 = 3.5f;
  var res : f32 = round(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_9edc38();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_9edc38();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_9edc38();
}
