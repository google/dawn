SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupExclusiveAdd_c08160() -> vec3<i32> {
  var res : vec3<i32> = subgroupExclusiveAdd(vec3<i32>(1i));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_c08160();
}

Failed to generate: error: Unknown builtin method: 0x55d88c4a4230
