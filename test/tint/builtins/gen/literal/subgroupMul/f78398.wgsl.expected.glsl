SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupMul_f78398() -> vec2<f32> {
  var res : vec2<f32> = subgroupMul(vec2<f32>(1.0f));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_f78398();
}

Failed to generate: error: Unknown builtin method: 0x5602e75bf230
