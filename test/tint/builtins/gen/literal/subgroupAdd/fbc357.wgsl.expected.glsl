SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupAdd_fbc357() -> vec4<u32> {
  var res : vec4<u32> = subgroupAdd(vec4<u32>(1u));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_fbc357();
}

Failed to generate: error: Unknown builtin method: 0x55c5105db230
