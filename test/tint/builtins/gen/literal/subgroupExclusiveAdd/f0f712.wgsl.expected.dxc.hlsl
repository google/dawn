SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupExclusiveAdd_f0f712() -> vec2<i32> {
  var res : vec2<i32> = subgroupExclusiveAdd(vec2<i32>(1i));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_f0f712();
}

Failed to generate: error: Unknown builtin method: subgroupExclusiveAdd
