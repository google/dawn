SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupAdd_1eb429() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupAdd(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_1eb429();
}

Failed to generate: error: Unknown builtin method: 0x55946b0af498
