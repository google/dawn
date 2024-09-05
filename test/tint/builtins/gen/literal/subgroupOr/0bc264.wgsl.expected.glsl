SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupOr_0bc264() -> u32 {
  var res : u32 = subgroupOr(1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_0bc264();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupOr/0bc264.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

