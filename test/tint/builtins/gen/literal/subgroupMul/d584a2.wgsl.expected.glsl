SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupMul_d584a2() -> vec2<i32> {
  var res : vec2<i32> = subgroupMul(vec2<i32>(1i));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_d584a2();
}

Failed to generate: error: Unknown builtin method: 0x55f27a9e7230
