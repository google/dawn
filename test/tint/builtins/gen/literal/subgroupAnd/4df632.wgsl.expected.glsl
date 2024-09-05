SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupAnd_4df632() -> u32 {
  var res : u32 = subgroupAnd(1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAnd_4df632();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAnd/4df632.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

