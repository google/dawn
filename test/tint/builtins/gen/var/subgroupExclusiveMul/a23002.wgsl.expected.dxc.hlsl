SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupExclusiveMul_a23002() -> i32 {
  var arg_0 = 1i;
  var res : i32 = subgroupExclusiveMul(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_a23002();
}

Failed to generate: error: Unknown builtin method: subgroupExclusiveMul
