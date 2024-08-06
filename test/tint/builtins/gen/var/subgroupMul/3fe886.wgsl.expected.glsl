SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupMul_3fe886() -> i32 {
  var arg_0 = 1i;
  var res : i32 = subgroupMul(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_3fe886();
}

Failed to generate: error: Unknown builtin method: 0x56028c5131c0
