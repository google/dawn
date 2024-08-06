SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupExclusiveAdd_42684c() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupExclusiveAdd(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_42684c();
}

Failed to generate: error: Unknown builtin method: subgroupExclusiveAdd
