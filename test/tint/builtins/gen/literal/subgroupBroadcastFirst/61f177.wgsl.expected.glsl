SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupBroadcastFirst_61f177() -> u32 {
  var res : u32 = subgroupBroadcastFirst(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcastFirst_61f177();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcastFirst_61f177();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcastFirst/61f177.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupBroadcastFirst_61f177() -> u32 {
  var res : u32 = subgroupBroadcastFirst(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcastFirst_61f177();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcastFirst_61f177();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcastFirst/61f177.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

