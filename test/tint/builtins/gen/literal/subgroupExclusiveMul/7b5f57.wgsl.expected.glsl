SKIP: INVALID


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupExclusiveMul_7b5f57() -> vec4<f32> {
  var res : vec4<f32> = subgroupExclusiveMul(vec4<f32>(1.0f));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_7b5f57();
}

Failed to generate: error: Unknown builtin method: 0x56269c636230
