SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupExclusiveAdd_71ad0f() -> vec4<f32> {
  var res : vec4<f32> = subgroupExclusiveAdd(vec4<f32>(1.0f));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_71ad0f();
}

Failed to generate: error: Unknown builtin method: 0x55f312d57230
