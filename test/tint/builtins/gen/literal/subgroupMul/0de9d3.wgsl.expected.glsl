SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupMul_0de9d3() -> f32 {
  var res : f32 = subgroupMul(1.0f);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_0de9d3();
}

Failed to generate: error: Unknown builtin method: 0x55802f025f58
