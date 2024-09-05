SKIP: INVALID


enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupExclusiveAdd_4a1568() -> f16 {
  var res : f16 = subgroupExclusiveAdd(1.0h);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_4a1568();
}

Failed to generate: error: Unknown builtin method: 0x561d89ec2f58
