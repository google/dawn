SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupMul_fab258() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = subgroupMul(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_fab258();
}

Failed to generate: error: Unknown builtin method: 0x55f7efd1d498
