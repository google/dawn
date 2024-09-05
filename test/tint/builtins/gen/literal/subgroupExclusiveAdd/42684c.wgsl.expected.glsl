SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupExclusiveAdd_42684c() -> u32 {
  var res : u32 = subgroupExclusiveAdd(1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_42684c();
}

Failed to generate: error: Unknown builtin method: 0x55cadd930f58
