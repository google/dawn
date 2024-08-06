SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupAdd_22d041() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupAdd(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_22d041();
}

Failed to generate: error: Unknown builtin method: 0x55fb1e161498
