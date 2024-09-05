SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupExclusiveMul_87f23e() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupExclusiveMul(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_87f23e();
}

Failed to generate: error: Unknown builtin method: 0x559158a7b498
