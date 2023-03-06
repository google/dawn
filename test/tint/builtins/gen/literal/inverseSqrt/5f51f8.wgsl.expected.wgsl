enable f16;

fn inverseSqrt_5f51f8() {
  var res : vec2<f16> = inverseSqrt(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_5f51f8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_5f51f8();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_5f51f8();
}
