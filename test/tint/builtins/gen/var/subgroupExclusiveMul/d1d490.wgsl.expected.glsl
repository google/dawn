SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupExclusiveMul_d1d490() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = subgroupExclusiveMul(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_d1d490();
}

Failed to generate: error: Unknown builtin method: 0x560cebc18498
