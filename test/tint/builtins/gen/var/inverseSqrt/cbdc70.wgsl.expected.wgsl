enable f16;

fn inverseSqrt_cbdc70() {
  var arg_0 = vec4<f16>(1.0h);
  var res : vec4<f16> = inverseSqrt(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_cbdc70();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_cbdc70();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_cbdc70();
}
