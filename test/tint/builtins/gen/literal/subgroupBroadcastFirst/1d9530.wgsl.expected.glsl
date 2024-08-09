SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupBroadcastFirst_1d9530() -> vec2<u32> {
  var res : vec2<u32> = subgroupBroadcastFirst(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcastFirst_1d9530();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcastFirst_1d9530();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcastFirst/1d9530.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupBroadcastFirst_1d9530() -> vec2<u32> {
  var res : vec2<u32> = subgroupBroadcastFirst(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcastFirst_1d9530();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcastFirst_1d9530();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcastFirst/1d9530.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

