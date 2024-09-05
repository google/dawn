SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupExclusiveMul_4525a3() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupExclusiveMul(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_4525a3();
}

Failed to generate: error: Unknown builtin method: 0x557707c4d498
