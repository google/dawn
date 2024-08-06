SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupAdd_b61df7() -> u32 {
  var res : u32 = subgroupAdd(1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_b61df7();
}

Failed to generate: error: Unknown builtin method: subgroupAdd
