enable f16;

fn inverseSqrt_440300() {
  var res : f16 = inverseSqrt(1.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_440300();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_440300();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_440300();
}
