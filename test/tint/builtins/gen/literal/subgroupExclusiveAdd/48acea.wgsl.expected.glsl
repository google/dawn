SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupExclusiveAdd_48acea() -> vec2<u32> {
  var res : vec2<u32> = subgroupExclusiveAdd(vec2<u32>(1u));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_48acea();
}

Failed to generate: error: Unknown builtin method: 0x55cb7c0f3230
