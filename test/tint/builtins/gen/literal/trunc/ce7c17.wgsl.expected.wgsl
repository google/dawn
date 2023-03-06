enable f16;

fn trunc_ce7c17() {
  var res : vec4<f16> = trunc(vec4<f16>(1.5h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_ce7c17();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_ce7c17();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_ce7c17();
}
