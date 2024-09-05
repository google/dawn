SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupExclusiveAdd_967e38() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = subgroupExclusiveAdd(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_967e38();
}

Failed to generate: error: Unknown builtin method: 0x55e4355c61c0
