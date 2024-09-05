SKIP: INVALID


enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupAdd_225207() -> f16 {
  var res : f16 = subgroupAdd(1.0h);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_225207();
}

Failed to generate: error: Unknown builtin method: 0x55ac4fb17f58
